/*
 * Copyright 2011 Intel Corporation.
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

static int sRetrievedMinimum = 0;

EmailAgent *EmailAgent::m_instance = 0;

EmailAgent *EmailAgent::instance()
{
    if (!m_instance)
        m_instance = new EmailAgent();
    return m_instance;
}

EmailAgent::EmailAgent(QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
    , m_retrieving(false)
    , m_transmitting(false)
    , m_exporting(false)
    , m_cancelling(false)
    , m_retrievalAction(new QMailRetrievalAction(this))
    , m_storageAction(new QMailStorageAction(this))
    , m_transmitAction(new QMailTransmitAction(this))
    , m_exportAction(new QMailRetrievalAction(this))
#ifdef HAS_MLITE
    , m_confirmDeleteMail(new MGConfItem("/apps/meego-app-email/confirmdeletemail"))
#endif
{
    initMailServer(); 

    connect(m_retrievalAction, SIGNAL(activityChanged(QMailServiceAction::Activity)),
            this, SLOT(activityChanged(QMailServiceAction::Activity)));

    connect(m_transmitAction, SIGNAL(progressChanged(uint, uint)),
            this, SLOT(progressChanged(uint,uint)));
    connect(m_transmitAction, SIGNAL(activityChanged(QMailServiceAction::Activity)),
            this, SLOT(activityChanged(QMailServiceAction::Activity)));
    connect(m_exportAction, SIGNAL(activityChanged(QMailServiceAction::Activity)),
            this, SLOT(exportActivityChanged(QMailServiceAction::Activity)));

    connect (QMailStore::instance(), SIGNAL(foldersAdded(const QMailFolderIdList &)), this,
             SLOT(onFoldersAdded(const QMailFolderIdList &)));

    // Set the default interval as 2 secs
    m_exportTimer.setInterval(2000);
    connect(&m_exportTimer, SIGNAL(timeout()), this, SLOT(exportAccounts()));

    m_instance = this;
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

EmailAgent::~EmailAgent()
{
    delete m_retrievalAction;
    delete m_storageAction;
    delete m_transmitAction;
}

void EmailAgent::accountsSync()
{
    if (!isSynchronizing()) 
    {
        // First we need to ensure that account lists are empty
        m_retrieveAccounts.clear();
        m_transmitAccounts.clear();

        // Get keys to avoid errors
        QMailAccountKey enabledKey(QMailAccountKey::status(QMailAccount::Enabled, QMailDataComparator::Includes));
        QMailAccountKey retrieveKey(QMailAccountKey::status(QMailAccount::CanRetrieve, QMailDataComparator::Includes));
        QMailAccountKey transmitKey(QMailAccountKey::status(QMailAccount::CanTransmit, QMailDataComparator::Includes));

        // Query accounts by capabilities
        m_retrieveAccounts = QMailStore::instance()->queryAccounts(enabledKey & retrieveKey);
        m_transmitAccounts = QMailStore::instance()->queryAccounts(enabledKey & transmitKey);
    }

    // Trigger accounts retrieving first
    if (!m_retrieveAccounts.isEmpty())
        synchronize(m_retrieveAccounts.takeFirst());
     else
     {
         emit syncCompleted();
     }
}

void EmailAgent::deleteMessage(QVariant id)
{
    QMailMessageId msgId = id.value<QMailMessageId>();
    QMailMessageIdList msgIdList;
    msgIdList << msgId;
    QMailMessage msg(msgId);
    exportAccountChanges(msg.parentAccountId());
    return deleteMessages (msgIdList);
}

void EmailAgent::deleteMessages(const QMailMessageIdList &ids)
{
    Q_ASSERT(!ids.empty());

    m_storageAction->deleteMessages(ids);
    emit messagesDeleted(ids);
}

void EmailAgent::createFolder (const QString &name, QVariant mailAccountId, QVariant parentFolderId)
{
    
    Q_ASSERT(!name.isEmpty());

    QMailAccountId accountId = mailAccountId.value<QMailAccountId>();
    Q_ASSERT(accountId.isValid());

    QMailFolderId parentId = parentFolderId.value<QMailFolderId>();

    m_storageAction->onlineCreateFolder(name, accountId, parentId);
}

void EmailAgent::deleteFolder(QVariant folderId)
{
    
    QMailFolderId id = folderId.value<QMailFolderId>();
    Q_ASSERT(id.isValid());

    m_storageAction->onlineDeleteFolder(id);
}

void EmailAgent::renameFolder(QVariant folderId, const QString &name)
{
    Q_ASSERT(!name.isEmpty());

    QMailFolderId id = folderId.value<QMailFolderId>();
    Q_ASSERT(id.isValid());

    m_storageAction->onlineRenameFolder(id, name);
}

void EmailAgent::synchronize(QVariant id)
{
    QMailAccountId accountId = id.value<QMailAccountId>();

    if (!isSynchronizing() && accountId.isValid()) {
        emit syncBegin();
        m_cancelling = false;
        m_retrieving = true;
        //m_retrievalAction->retrieveAll(accountId);
        m_retrievalAction->synchronize(accountId, 20);
    }
}

void EmailAgent::getMoreMessages(QVariant vFolderId)
{
    QMailFolderId folderId = vFolderId.value<QMailFolderId>();
    if (!isSynchronizing() && folderId.isValid()) {
        emit syncBegin();
        QMailFolder folder(folderId);
        QMailMessageKey countKey(QMailMessageKey::parentFolderId(folderId));
        countKey &= ~QMailMessageKey::status(QMailMessage::Temporary);
        sRetrievedMinimum = QMailStore::instance()->countMessages(countKey);
        sRetrievedMinimum += 20;
        m_cancelling = false;
        m_retrieving = true;
        //m_retrievalAction->retrieveMessageList(folder.parentAccountId(), folderId, sRetrievedMinimum);
        m_retrievalAction->synchronize(folder.parentAccountId(), sRetrievedMinimum);
    }
}

void EmailAgent::sendMessages(const QMailAccountId &id)
{
    if (!isSynchronizing() && id.isValid()) {
        m_cancelling = false;
        m_transmitting = true;
        m_transmitAction->transmitMessages(id);
    }
}

void EmailAgent::progressChanged(uint value, uint total)
{
    qDebug() << "progress " << value << " of " << total;
    int percent = (value * 100) / total;
    emit progressUpdate (percent);
}

void EmailAgent::activityChanged(QMailServiceAction::Activity activity)
{
    QMailServiceAction *action = static_cast<QMailServiceAction*>(sender());
    const QMailServiceAction::Status status(action->status());

    switch (activity) {
        case QMailServiceAction::Failed:

            // don't try to synchronise extra accounts if the user cancelled the sync
            if (m_cancelling) {
                m_retrieving = false;
                m_transmitting = false;
                emit error(status.accountId, status.text, status.errorCode);
                return;
            } else {
                // Report the error
                emit error(status.accountId, status.text, status.errorCode);
            }
            // fallthrough; if this wasn't a fatal error, try sync again
        case QMailServiceAction::Successful:
            if (action == m_retrievalAction) {
                m_retrieving = false;
                if (m_retrieveAccounts.isEmpty()) {
                    emit retrievalCompleted();
                    // If there's no account to retrieve let's start sending msgs
                    if (!m_transmitAccounts.isEmpty())
                        sendMessages(m_transmitAccounts.takeFirst());
                }
                else
                    synchronize(m_retrieveAccounts.takeFirst());
            }
            else if (action == m_transmitAction) {
                m_transmitting = false;
                if (m_transmitAccounts.isEmpty())
                {
                    emit sendCompleted();
                }
                else
                    sendMessages(m_transmitAccounts.takeFirst());
            }

            if (!isSynchronizing())
            {
                emit syncCompleted();
            }

        default:
            qDebug() << "Activity State Changed:" << activity;
            break;
    }
}

bool EmailAgent::isSynchronizing() const
{
    return (m_retrieving || m_transmitting);
}

void EmailAgent::cancelSync()
{
    if (!isSynchronizing())
        return;

    m_cancelling = true;
    // there's an assert on isValid (essentially is running) inside
    // QMailServiceAction. we should fix this there, but for now, work around
    // it.
    if (m_transmitAction->isRunning())
        m_transmitAction->cancelOperation();
    if (m_retrievalAction->isRunning())
        m_retrievalAction->cancelOperation();
}

void EmailAgent::markMessageAsRead(QVariant msgId)
{
    QMailMessageId id = msgId.value<QMailMessageId>();
    quint64 status(QMailMessage::Read);
    QMailStore::instance()->updateMessagesMetaData(QMailMessageKey::id(id), status, true);
    QMailMessage msg (id);
    exportAccountChanges(msg.parentAccountId());
}

void EmailAgent::markMessageAsUnread(QVariant msgId)
{
    QMailMessageId id = msgId.value<QMailMessageId>();
    quint64 status(QMailMessage::Read);
    QMailStore::instance()->updateMessagesMetaData(QMailMessageKey::id(id), status, false);
    QMailMessage msg (id);
    exportAccountChanges(msg.parentAccountId());
}

QString EmailAgent::getSignatureForAccount(QVariant vMailAccountId)
{
    QMailAccountId mailAccountId = vMailAccountId.value<QMailAccountId>();
    if (mailAccountId.isValid())
    {
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

void EmailAgent::exportAccountChanges(const QMailAccountId id)
{
    // Only add the account to the list if it's not
    // already there.
    if (id.isValid() &&
        !m_exportAccounts.contains(id)) {

        // Add account to the list
        m_exportAccounts.append(id);

        // Start the timer!
        if (!m_exportTimer.isActive() && !m_exporting)
            m_exportTimer.start();
    }

}

void EmailAgent::exportAccounts()
{

    // Mark as exporting if it's not
    if (!m_exporting) {
        m_exporting = true;
        m_exportTimer.stop();
    }

    // Export message updates
    if (!m_exportAccounts.isEmpty())
        m_exportAction->exportUpdates(m_exportAccounts.first());
}

void EmailAgent::exportActivityChanged(QMailServiceAction::Activity  activity)
{
    QMailServiceAction *action = static_cast<QMailServiceAction*>(sender());
    const QMailServiceAction::Status status(action->status());

    if (action && m_exporting) {
        if (activity == QMailServiceAction::Successful) {
            m_exporting = false;
            m_exportAccounts.removeFirst();

            // Check if there's more accounts to export
            if (!m_exportAccounts.isEmpty())
                exportAccounts();
        }
    }
}


// callback function to handle foldersAdded signal
void EmailAgent::onFoldersAdded (const QMailFolderIdList & folderIdList)
{
    QMailFolder folder(folderIdList[0]);
    QMailAccountId accountId = folder.parentAccountId();

    synchronize (accountId);
}

void EmailAgent::downloadAttachment(QVariant vMsgId, const QString & attachmentDisplayName)
{
    m_messageId = vMsgId.value<QMailMessageId>();
    QMailMessage message (m_messageId);

    emit attachmentDownloadStarted();
            qDebug()<<"PartsCount: "<<message.partCount();
            qDebug()<<"attachmentDisplayName: "<<attachmentDisplayName;
    for (uint i = 1; i < message.partCount(); i++)
    {
        QMailMessagePart sourcePart = message.partAt(i);
        if (attachmentDisplayName == sourcePart.displayName())
        {
            QMailMessagePart::Location location = sourcePart.location();
            location.setContainingMessageId(m_messageId);
            if (sourcePart.hasBody())
            {
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
    if (QMailServiceAction *action = static_cast<QMailServiceAction*>(sender()))
    {
        if (activity == QMailServiceAction::Successful)
        {
            if (action == m_attachmentRetrievalAction)
            {
                QMailMessage message(m_messageId);
                for (uint i = 1; i < message.partCount(); ++i)
                {
                    const QMailMessagePart &sourcePart(message.partAt(i));
                    if (sourcePart.location().toString(false) == m_attachmentPart.location().toString(false))
                    {
                        QString downloadFolder = QDir::homePath() + "/Downloads/";
                        sourcePart.writeBodyTo(downloadFolder);
                        emit attachmentDownloadCompleted();
                    }
                }
            }
        }
        else if (activity == QMailServiceAction::Failed)
        {
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
    if (s.contains("video", Qt::CaseInsensitive))
    {
        parameters << "--app" << "meego-app-video";
        parameters << "--cmd" << "playVideo";
    }
    else if (s.contains("image", Qt::CaseInsensitive))
    {
        parameters << "--app" << "meego-app-photos";
        parameters << "--cmd" << "showPhoto";
    }
    else if (s.contains("audio", Qt::CaseInsensitive))
    {
        parameters << "--app" << "meego-app-music";
        parameters << "--cmd" << "playSong";
    }
    else if (s.contains("Ogg data", Qt::CaseInsensitive))
    {
        // Fix Me:  need more research on Ogg format. For now, default to video.
        parameters << "--app" << "meego-app-video";
        parameters << "--cmd" << "video";
    }
    else
    {
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

    m_storageAction->flagMessages(ids, setMask, unsetMask);
}

QString EmailAgent::getMessageBodyFromFile (const QString& bodyFilePath)
{
    QFile f(bodyFilePath);
    if(!f.open(QFile::ReadOnly))
        return ("");

    QString data = f.readAll();
    return data;
}
