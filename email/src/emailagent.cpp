/*
 * Copyright 2011 Intel Corporation.
 * Copyright (C) 2012 Jolla Ltd.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 	
 * http://www.apache.org/licenses/LICENSE-2.0
 */


#include <QTimer>
#include <QDir>
#include <QUrl>
#include <QFile>
#include <QProcess>

#include <qmailnamespace.h>
#include <qmailaccount.h>
#include <qmailstore.h>

#include "emailagent.h"
#include "emailaction.h"

namespace {

QMailAccountId accountForMessageId(const QMailMessageId &msgId)
{
    QMailMessageMetaData metaData(msgId);
    return metaData.parentAccountId();
}
}

EmailAgent *EmailAgent::m_instance = 0;

EmailAgent *EmailAgent::instance()
{
    if (!m_instance)
        m_instance = new EmailAgent();
    return m_instance;
}

EmailAgent::EmailAgent(QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
    , m_actionCount(0)
    , m_retrieving(false)
    , m_transmitting(false)
    , m_cancelling(false)
    , m_synchronizing(false)
    , m_retrievalAction(new QMailRetrievalAction(this))
    , m_storageAction(new QMailStorageAction(this))
    , m_transmitAction(new QMailTransmitAction(this))
#ifdef HAS_MLITE
    , m_confirmDeleteMail(new MGConfItem("/apps/meego-app-email/confirmdeletemail"))
#endif
{
    initMailServer(); 

    connect(m_transmitAction.data(), SIGNAL(progressChanged(uint, uint)),
            this, SLOT(progressChanged(uint,uint)));

    connect(m_retrievalAction.data(), SIGNAL(activityChanged(QMailServiceAction::Activity)),
            this, SLOT(activityChanged(QMailServiceAction::Activity)));

    connect(m_storageAction.data(), SIGNAL(activityChanged(QMailServiceAction::Activity)),
            this, SLOT(activityChanged(QMailServiceAction::Activity)));

    connect(m_transmitAction.data(), SIGNAL(activityChanged(QMailServiceAction::Activity)),
            this, SLOT(activityChanged(QMailServiceAction::Activity)));


    connect (QMailStore::instance(), SIGNAL(foldersAdded(const QMailFolderIdList &)), this,
             SLOT(onFoldersAdded(const QMailFolderIdList &)));

    m_instance = this;
}

EmailAgent::~EmailAgent()
{
}

quint64 EmailAgent::newAction()
{
    return quint64(++m_actionCount);
}

void EmailAgent::initMailServer()
{
    // starts the messageserver if it is not already running.

    QString lockfile = "messageserver-instance.lock";
    int id = QMail::fileLock(lockfile);
    if (id == -1) {
        // Server is currently running
        return;
    }
    QMail::fileUnlock(id);

    m_messageServerProcess.start("/usr/bin/messageserver");
    m_messageServerProcess.waitForStarted();
    return;
}

void EmailAgent::activityChanged(QMailServiceAction::Activity activity)
{
    QMailServiceAction *action = static_cast<QMailServiceAction*>(sender());
    const QMailServiceAction::Status status(action->status());

    switch (activity) {
    case QMailServiceAction::Failed:
        //TODO: coordinate with stop logic
        // don't try to synchronise extra accounts if the user cancelled the sync
        if (m_cancelling) {
            m_synchronizing = false;
            _actionQueue.clear();
            emit error(status.accountId, status.text, status.errorCode);
            break;
        } else {
            // Report the error
            dequeue();
             _currentAction = getNext();
            emit error(status.accountId, status.text, status.errorCode);

            if (_currentAction.isNull()) {
                qDebug() << "Sync completed with Errors!!!.";
                m_synchronizing = false;
                emit syncCompleted();
            }
            else {
                executeCurrent();
            }
            break;
        }

    case QMailServiceAction::Successful:
        dequeue();
        //TODO
        //more than one send action can occur at time
        //sendCompleted should also have accountId
        if (action == m_transmitAction.data()) {
            m_transmitting = false;
            emit sendCompleted();
        }

        _currentAction = getNext();

        if (_currentAction.isNull()) {
            qDebug() << "Sync completed.";
            m_synchronizing = false;
            emit syncCompleted();
        }
        else {
            executeCurrent();
        }
        break;

    default:
        //emit acctivity changed here
        qDebug() << "Activity State Changed:" << activity;
        break;
    }
}

bool EmailAgent::isSynchronizing() const
{
    return m_synchronizing;
}

/*
  Actions
*/

//Sync all accounts (both ways)
void EmailAgent::accountsSync(const bool syncOnlyInbox, const uint minimum)
{
    m_enabledAccounts.clear();
    QMailAccountKey enabledAccountKey = QMailAccountKey::status(QMailAccount::Enabled |
                                                         QMailAccount::CanRetrieve |
                                                         QMailAccount::CanTransmit,
                                                         QMailDataComparator::Includes);
    m_enabledAccounts = QMailStore::instance()->queryAccounts(enabledAccountKey);

    foreach (QMailAccountId accountId, m_enabledAccounts) {
        if (syncOnlyInbox) {
            QMailAccount account(accountId);
            QMailFolderId foldId = account.standardFolder(QMailFolder::InboxFolder);
            if (foldId.isValid()) {
                enqueue(new ExportUpdates(m_retrievalAction.data(),accountId));
                enqueue(new RetrieveFolderList(m_retrievalAction.data(), accountId, QMailFolderId(), true));
                enqueue(new RetrieveMessageList(m_retrievalAction.data(), accountId, foldId, minimum));
            }
            else {
                qDebug() << "Error: Inbox folder not found for account:" << accountId.toULongLong();
            }
        }
        else {
            enqueue(new Synchronize(m_retrievalAction.data(), accountId));
        }
    }
}

//Add logic to move to trash only and fully delete from trash
void EmailAgent::deleteMessage(QVariant id)
{
    QMailMessageId msgId = id.value<QMailMessageId>();
    QMailMessageIdList msgIdList;
    msgIdList << msgId;
    deleteMessages (msgIdList);
}

void EmailAgent::deleteMessages(const QMailMessageIdList &ids)
{
    Q_ASSERT(!ids.empty());
    enqueue(new DeleteMessages(m_storageAction.data(), ids));
}

void EmailAgent::createFolder(const QString &name, QVariant mailAccountId, QVariant parentFolderId)
{
    
    if(!name.isEmpty()) {
        qDebug() << "Error: Can't create a folder with empty name";
    }

    else {
        QMailAccountId accountId = mailAccountId.value<QMailAccountId>();
        Q_ASSERT(accountId.isValid());

        QMailFolderId parentId = parentFolderId.value<QMailFolderId>();

        enqueue(new OnlineCreateFolder(m_storageAction.data(), name, accountId, parentId));
    }
}

void EmailAgent::deleteFolder(QVariant folderId)
{    
    QMailFolderId id = folderId.value<QMailFolderId>();
    Q_ASSERT(id.isValid());

    enqueue(new OnlineDeleteFolder(m_storageAction.data(),id));
}

void EmailAgent::renameFolder(QVariant folderId, const QString &name)
{
    if(!name.isEmpty()) {
        qDebug() << "Error: Can't rename a folder to a empty name";
    }

    else{
        QMailFolderId id = folderId.value<QMailFolderId>();
        Q_ASSERT(id.isValid());

        enqueue(new OnlineRenameFolder(m_storageAction.data(),id, name));
    }
}

void EmailAgent::retrieveFolderList(QVariant accountId, QVariant folderId, const bool descending)
{
    QMailAccountId acctId = accountId.value<QMailAccountId>();
    QMailFolderId foldId = folderId.value<QMailFolderId>();

    if (acctId.isValid()) {
        emit syncBegin(); //check
        m_cancelling = false;
        m_retrieving = true;
        enqueue(new RetrieveFolderList(m_retrievalAction.data(),acctId, foldId, descending));
    }
}

void EmailAgent::synchronize(QVariant id)
{
    QMailAccountId accountId = id.value<QMailAccountId>();

    if (!isSynchronizing() && accountId.isValid()) {
        emit syncBegin(); //check
        m_cancelling = false;
        m_retrieving = true;
        enqueue(new Synchronize(m_retrievalAction.data(), accountId));
    }
}

void EmailAgent::retrieveMessageList(QVariant accountId, QVariant folderId, const uint minimum)
{
    QMailAccountId acctId = accountId.value<QMailAccountId>();
    QMailFolderId foldId = folderId.value<QMailFolderId>();

    if (acctId.isValid()) {
        emit syncBegin(); //check
        m_cancelling = false; //check
        m_retrieving = true; //check
        enqueue(new RetrieveMessageList(m_retrievalAction.data(),acctId, foldId, minimum));
    }
}

void EmailAgent::getMoreMessages(QVariant vFolderId, uint minimum)
{
    QMailFolderId folderId = vFolderId.value<QMailFolderId>();
    if (folderId.isValid()) {
        emit syncBegin(); //check
        m_cancelling = false; //check
        m_retrieving = true; //check
        QMailFolder folder(folderId);
        QMailMessageKey countKey(QMailMessageKey::parentFolderId(folderId));
        countKey &= ~QMailMessageKey::status(QMailMessage::Temporary);
        minimum += QMailStore::instance()->countMessages(countKey);
        enqueue(new RetrieveMessageList(m_retrievalAction.data(), folder.parentAccountId(), folderId, minimum));
    }
}

//messages should be moved to outbox, and marked as local only
//since device can be offline at the point user tries to send
//the mail.
void EmailAgent::sendMessages(const QMailAccountId &id)
{
    if (id.isValid()) {
        m_cancelling = false; //check
        m_transmitting = true; //check
        enqueue(new TransmitMessages(m_transmitAction.data(),id));
    }
}

void EmailAgent::progressChanged(uint value, uint total)
{
    qDebug() << "progress " << value << " of " << total;
    int percent = (value * 100) / total;
    emit progressUpdate (percent);
}

void EmailAgent::cancelSync()
{
    if (!isSynchronizing())
        return;

    m_cancelling = true;

    //clear the actions queue
    _actionQueue.clear();

    //cancel running action
    if (((_currentAction->serviceAction)->activity() == QMailServiceAction::Pending ||
        (_currentAction->serviceAction)->activity() == QMailServiceAction::InProgress)) {
        (_currentAction->serviceAction)->cancelOperation();
    }
}

void EmailAgent::markMessageAsRead(QVariant msgId)
{
    QMailMessageId id = msgId.value<QMailMessageId>();
    quint64 status(QMailMessage::Read);
    QMailStore::instance()->updateMessagesMetaData(QMailMessageKey::id(id), status, true);
    exportUpdates(accountForMessageId(id));
}

void EmailAgent::markMessageAsUnread(QVariant msgId)
{
    QMailMessageId id = msgId.value<QMailMessageId>();
    quint64 status(QMailMessage::Read);
    QMailStore::instance()->updateMessagesMetaData(QMailMessageKey::id(id), status, false);
    exportUpdates(accountForMessageId(id));
}

QString EmailAgent::getSignatureForAccount(QVariant vMailAccountId)
{
    QMailAccountId mailAccountId = vMailAccountId.value<QMailAccountId>();
    if (mailAccountId.isValid()) {
        QMailAccount mailAccount (mailAccountId);
        return mailAccount.signature();
    }
    return (QString (""));
}

bool EmailAgent::confirmDeleteMail()
{
#ifdef HAS_MLITE
    return m_confirmDeleteMail->value().toBool();
#else
    return true;
#endif
}

void EmailAgent::exportUpdates(const QMailAccountId id)
{
    enqueue(new ExportUpdates(m_retrievalAction.data(),id));
}

// callback function to handle foldersAdded signal
void EmailAgent::onFoldersAdded (const QMailFolderIdList & folderIdList)
{
    QMailFolder folder(folderIdList[0]);
    QMailAccountId accountId = folder.parentAccountId();
    //TODO: Check if this is necessary here
    synchronize(accountId);
}

void EmailAgent::downloadAttachment(QVariant vMsgId, const QString & attachmentDisplayName)
{
    m_messageId = vMsgId.value<QMailMessageId>();
    QMailMessage message (m_messageId);

    emit attachmentDownloadStarted();
    qDebug()<<"PartsCount: "<<message.partCount();
    qDebug()<<"attachmentDisplayName: "<<attachmentDisplayName;

    for (uint i = 1; i < message.partCount(); i++) {
        QMailMessagePart sourcePart = message.partAt(i);
        if (attachmentDisplayName == sourcePart.displayName()) {
            QMailMessagePart::Location location = sourcePart.location();
            location.setContainingMessageId(m_messageId);
            if (sourcePart.hasBody()) {
                // The content has already been downloaded to local device.
                QString downloadFolder = QDir::homePath() + "/Downloads/";
                QFile f(downloadFolder+sourcePart.displayName());
                if (f.exists())
                    f.remove();
                sourcePart.writeBodyTo(downloadFolder);
                emit attachmentDownloadCompleted();
                return;
            }
            m_attachmentPart = sourcePart;
            m_attachmentRetrievalAction = new QMailRetrievalAction(this);
            connect (m_attachmentRetrievalAction, SIGNAL(activityChanged(QMailServiceAction::Activity)),
                     this, SLOT(attachmentDownloadActivityChanged(QMailServiceAction::Activity)));
            connect(m_attachmentRetrievalAction, SIGNAL(progressChanged(uint, uint)),
                    this, SLOT(progressChanged(uint,uint)));

            m_attachmentRetrievalAction->retrieveMessagePart(location);
        }
    }
}


void EmailAgent::attachmentDownloadActivityChanged(QMailServiceAction::Activity activity)
{
    if (QMailServiceAction *action = static_cast<QMailServiceAction*>(sender())) {
        if (activity == QMailServiceAction::Successful) {
            if (action == m_attachmentRetrievalAction) {
                QMailMessage message(m_messageId);
                for (uint i = 1; i < message.partCount(); ++i) {
                    const QMailMessagePart &sourcePart(message.partAt(i));
                    if (sourcePart.location().toString(false) == m_attachmentPart.location().toString(false)) {
                        QString downloadFolder = QDir::homePath() + "/Downloads/";
                        sourcePart.writeBodyTo(downloadFolder);
                        emit attachmentDownloadCompleted();
                    }
                }
            }
        }
        else if (activity == QMailServiceAction::Failed) {
            emit attachmentDownloadCompleted();
        }
    }
}

bool EmailAgent::openAttachment(const QString & uri)
{
    bool status = true;
    
    // let's determine the file type
    QString filePath = QDir::homePath() + "/Downloads/" + uri;

    QProcess fileProcess;
    fileProcess.start("file", QStringList() << "-b" << filePath);
    if (!fileProcess.waitForFinished())
        return false;

    QString s(fileProcess.readAll());
    QStringList parameters;
    parameters << "--opengl" << "--fullscreen";
    if (s.contains("video", Qt::CaseInsensitive)) {
        parameters << "--app" << "meego-app-video";
        parameters << "--cmd" << "playVideo";
    }
    else if (s.contains("image", Qt::CaseInsensitive)) {
        parameters << "--app" << "meego-app-photos";
        parameters << "--cmd" << "showPhoto";
    }
    else if (s.contains("audio", Qt::CaseInsensitive)) {
        parameters << "--app" << "meego-app-music";
        parameters << "--cmd" << "playSong";
    }
    else if (s.contains("Ogg data", Qt::CaseInsensitive)) {
        // Fix Me:  need more research on Ogg format. For now, default to video.
        parameters << "--app" << "meego-app-video";
        parameters << "--cmd" << "video";
    }
    else {
        // Unsupported file type.
        return false;
    }

    QString executable("meego-qml-launcher");
    filePath.prepend("file://");
    parameters << "--cdata" << filePath;
    QProcess::startDetached(executable, parameters);

    return status;
}

void EmailAgent::openBrowser(const QString & url)
{
    QString executable("xdg-open");
    QStringList parameters;
    parameters << url;
    QProcess::startDetached(executable, parameters);
}

void EmailAgent::flagMessages(const QMailMessageIdList &ids, quint64 setMask,
        quint64 unsetMask)
{
    Q_ASSERT(!ids.empty());

    enqueue(new FlagMessages(m_storageAction.data(), ids, setMask, unsetMask));
}

QString EmailAgent::getMessageBodyFromFile (const QString& bodyFilePath)
{
    QFile f(bodyFilePath);
    if(!f.open(QFile::ReadOnly))
        return ("");

    QString data = f.readAll();
    return data;
}

/*
  Queue of actions
*/

void EmailAgent::enqueue(EmailAction *actionPointer)
{
    Q_ASSERT(actionPointer);
    QSharedPointer<EmailAction> action(actionPointer);
    bool foundAction = actionInQueue(action);

   // Add checks for network availablity if online action

    if (!foundAction) {
        // It's a new action.
        action->id = newAction();
        _actionQueue.append(action);

        if (_currentAction.isNull()) {
            // Nothin is running, start this one immdetiately.
            _currentAction = action;
            executeCurrent();
        }
    }
    else {
        qWarning() << "This request already exists in the queue: " << action->description();
        qDebug() << "Number of actions in the queue: " << _actionQueue.size();
    }
}

void EmailAgent::dequeue()
{
    if(_actionQueue.isEmpty()) {
        qDebug() << "Error: can't dequeue emtpy list";
    }
    else {
        _actionQueue.removeFirst();
    }
}

bool EmailAgent::actionInQueue(QSharedPointer<EmailAction> action) const
{
    //check current first, there's chances that
    //user taps same action several times.
    if (!_currentAction.isNull()
        && *(_currentAction.data()) == *(action.data())) {
        return true;
    }
    else {
        foreach (const QSharedPointer<EmailAction> &a, _actionQueue) {
            if (*(a.data()) == *(action.data())) {
                return true;
            }
        }
    }
    return false;
}

void EmailAgent::executeCurrent()
{
    Q_ASSERT (!_currentAction.isNull());

    if(!m_synchronizing)
        m_synchronizing = true;

    //add network and qCop checks here.
    qDebug() << "Executing " << _currentAction->description();

    _currentAction->execute();
}

QSharedPointer<EmailAction> EmailAgent::getNext()
{
    if(_actionQueue.isEmpty())
        return QSharedPointer<EmailAction>();

    return _actionQueue.first();
}

