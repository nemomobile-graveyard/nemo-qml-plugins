/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 	
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <QDateTime>
#include <QTimer>
#include <QProcess>

#include <qmailmessage.h>
#include <qmailmessagekey.h>
#include <qmailstore.h>
#include <qmailserviceaction.h>

#include <qmailnamespace.h>

#include "emailmessagelistmodel.h"

QString EmailMessageListModel::bodyHtmlText(QMailMessagePartContainer *container) const
{
    QMailMessageContentType contentType = container->contentType();

    if (container->multipartType() == QMailMessagePartContainerFwd::MultipartNone) {
        if (contentType.subType().toLower() == "html") {
            if (container->hasBody() && container->body().data().size() > 1) {
                return container->body().data();
            }
            else {
                connect (m_retrievalAction, SIGNAL(activityChanged(QMailServiceAction::Activity)),
                                            this, SLOT(downloadActivityChanged(QMailServiceAction::Activity)));
                QMailMessage *msg = (QMailMessage *)container;
                QMailMessageIdList ids;
                ids << msg->id();
                m_retrievalAction->retrieveMessages(ids, QMailRetrievalAction::Content);
                return " ";  // Put a space here as a place holder to notify UI that we do have html body.
                             // Should find a better way.
            }
        }
        return "";
    }

    if (!container->contentAvailable()) {
        // if content is not available, attempts to downlaod from the server.
        connect (m_retrievalAction, SIGNAL(activityChanged(QMailServiceAction::Activity)),
                                            this, SLOT(downloadActivityChanged(QMailServiceAction::Activity)));
        QMailMessage *msg = (QMailMessage *)container;
        QMailMessageIdList ids;
        ids << msg->id();
        m_retrievalAction->retrieveMessages(ids, QMailRetrievalAction::Content);
        return " ";  // Put a space here as a place holder to notify UI that we do have html body.
    }

    QString text("");
    for ( uint i = 0; i < container->partCount(); i++ ) {
        QMailMessagePart messagePart = container->partAt(i);
        contentType = messagePart.contentType();
        if (contentType.type().toLower() == "text" && contentType.subType().toLower() == "html") {
            if (messagePart.hasBody()) {
                text += messagePart.body().data();
            }
            else {
                connect (m_retrievalAction, SIGNAL(activityChanged(QMailServiceAction::Activity)),
                                            this, SLOT(downloadActivityChanged(QMailServiceAction::Activity)));

                QMailMessagePart::Location location = messagePart.location();
                m_retrievalAction->retrieveMessagePart(location);
                text = " ";
                break;
            }
        }
        QMailMessagePart subPart;
        for (uint j = 0; j < messagePart.partCount(); j++) {
            subPart = messagePart.partAt(j);
            contentType = subPart.contentType();
            if (contentType.type().toLower() == "text" && contentType.subType().toLower() == "html") {
                if (subPart.hasBody()) {
                    text += subPart.body().data();
                }
                else {
                    connect (m_retrievalAction, SIGNAL(activityChanged(QMailServiceAction::Activity)),
                                                this, SLOT(downloadActivityChanged(QMailServiceAction::Activity)));
                    QMailMessagePart::Location location = subPart.location();
                    m_retrievalAction->retrieveMessagePart(location);
                    text = " ";
                    break;
                }
            }
        }
    }
    return text;
}

QString EmailMessageListModel::bodyPlainText(const QMailMessage &mailMsg) const
{
    QMailMessagePartContainer *container = (QMailMessagePartContainer *)&mailMsg;
    QMailMessageContentType contentType = container->contentType();
    if (container->hasBody() && contentType.type().toLower() == "text" &&
            contentType.subType().toLower() == "plain") {
        return container->body().data();
    }

    QString text("");
    for ( uint i = 0; i < container->partCount(); i++ ) {
        QMailMessagePart messagePart = container->partAt(i);

        contentType = messagePart.contentType();
        if (messagePart.hasBody() && contentType.type().toLower() == "text" &&
                                     contentType.subType().toLower() == "plain") {
            text += messagePart.body().data() + "\n";
        }
        QMailMessagePart subPart;
        for (uint j = 0; j < messagePart.partCount(); j++) {
            subPart = messagePart.partAt(j);
            contentType = subPart.contentType();
            if (subPart.hasBody() && contentType.type().toLower() == "text" &&
                                     contentType.subType().toLower() == "plain") {
                text += subPart.body().data() + "\n";
            }
        }
    }
    return text;
}

//![0]
EmailMessageListModel::EmailMessageListModel(QObject *parent)
    : QMailMessageListModel(parent),
      m_retrievalAction(new QMailRetrievalAction(this)),
      m_storageAction(new QMailStorageAction(this))
{
    QHash<int, QByteArray> roles;
    roles[QMailMessageModelBase::MessageAddressTextRole] = "sender";
    roles[QMailMessageModelBase::MessageSubjectTextRole] = "subject";
    roles[QMailMessageModelBase::MessageFilterTextRole] = "messageFilter";
    roles[QMailMessageModelBase::MessageTimeStampTextRole] = "timeStamp";
    roles[QMailMessageModelBase::MessageSizeTextRole] = "size";
    roles[QMailMessageModelBase::MessageTypeIconRole] = "icon";
    roles[QMailMessageModelBase::MessageStatusIconRole] = "statusIcon";
    roles[QMailMessageModelBase::MessageDirectionIconRole] = "directionIcon";
    roles[QMailMessageModelBase::MessagePresenceIconRole] = "presenceIcon";
    roles[QMailMessageModelBase::MessageBodyTextRole] = "body";
    roles[QMailMessageModelBase::MessageIdRole] = "messageId";
    roles[MessageAttachmentCountRole] = "numberOfAttachments";
    roles[MessageAttachmentsRole] = "listOfAttachments";
    roles[MessageRecipientsRole] = "recipients";
    roles[MessageRecipientsDisplayNameRole] = "recipientsDisplayName";
    roles[MessageReadStatusRole] = "readStatus";
    roles[MessageHtmlBodyRole] = "htmlBody";
    roles[MessageQuotedBodyRole] = "quotedBody";
    roles[MessageUuidRole] = "messageUuid";
    roles[MessageSenderDisplayNameRole] = "senderDisplayName";
    roles[MessageSenderEmailAddressRole] = "senderEmailAddress";
    roles[MessageCcRole] = "cc";
    roles[MessageBccRole] = "bcc";
    roles[MessageTimeStampRole] = "qDateTime";
    roles[MessageSelectModeRole] = "selected";
    setRoleNames(roles);

    initMailServer();
    QMailAccountIdList ids = QMailStore::instance()->queryAccounts(
            QMailAccountKey::status(QMailAccount::Enabled, QMailDataComparator::Includes),
            QMailAccountSortKey::name());

    QMailMessageKey accountKey = QMailMessageKey::parentAccountId(ids);
    QMailMessageListModel::setKey(accountKey);
    m_key = key();
    QMailMessageSortKey sortKey = QMailMessageSortKey::timeStamp(Qt::DescendingOrder);
    QMailMessageListModel::setSortKey(sortKey);
    m_selectedMsgIds.clear();
}

EmailMessageListModel::~EmailMessageListModel()
{
    delete m_retrievalAction;
    delete m_storageAction;
}

int EmailMessageListModel::rowCount(const QModelIndex & parent) const {
    return QMailMessageListModel::rowCount(parent);
}

QVariant EmailMessageListModel::data(const QModelIndex & index, int role) const {
    if (!index.isValid() || index.row() > rowCount(parent(index)))
        return QVariant();

    QMailMessageId msgId = idFromIndex(index);

    if (role == QMailMessageModelBase::MessageTimeStampTextRole) {
        QMailMessageMetaData message(msgId);
        QDateTime timeStamp = message.date().toLocalTime();
        return (timeStamp.toString("hh:mm MM/dd/yyyy"));
    }
    else if (role == MessageAttachmentCountRole) {
        // return number of attachments
        QMailMessageMetaData messageMetaData(msgId);
        if (!messageMetaData.status() & QMailMessageMetaData::HasAttachments)
            return 0;

        QMailMessage message(msgId);
        int numberOfAttachments = 0;

        // TODO: This only considers the first level of parts - it won't work for unusual
        // MIME compositions, or messages where the attachments are inside a nested
        // message which has been forwarded, for example.

        // Also, it should probably be looking at contentDisposition rather than contentType.

        for (uint i = 1; i < message.partCount(); i++) {
            QMailMessagePart sourcePart = message.partAt(i);
            if (!(sourcePart.multipartType() == QMailMessagePartContainer::MultipartNone))
                continue;

            QMailMessageContentType contentType = sourcePart.contentType();
            if (sourcePart.hasBody() && contentType.type().toLower() == "text" &&
                                     contentType.subType().toLower() == "plain") {
                continue;
            }
            if (i == 1 && contentType.type().toLower() == "text" && contentType.subType().toLower() == "html")
                continue;

            numberOfAttachments += 1;
        }
        return numberOfAttachments;
    }
    else if (role == MessageAttachmentsRole) {
        // return a stringlist of attachments
        QMailMessageMetaData messageMetaData(msgId);
        if (!messageMetaData.status() & QMailMessageMetaData::HasAttachments)
            return QStringList();

        QMailMessage message(msgId);
        QStringList attachments;
        for (uint i = 1; i < message.partCount(); i++) {
            QMailMessagePart sourcePart = message.partAt(i);
            if (!(sourcePart.multipartType() == QMailMessagePartContainer::MultipartNone))
                continue;

            QMailMessageContentType contentType = sourcePart.contentType();
            if (sourcePart.hasBody() && contentType.type().toLower() == "text" &&
                                     contentType.subType().toLower() == "plain") {
                continue;
            }
            if (i == 1 && contentType.type().toLower() == "text" &&
                        contentType.subType().toLower() == "html") {
                continue;
            }
            attachments << sourcePart.displayName();
        }

        return attachments;
    }
    else if (role == MessageRecipientsRole) {
        QMailMessageMetaData messageMetaData(msgId);
        QStringList recipients;
        QList<QMailAddress> addresses = messageMetaData.recipients();
        foreach (const QMailAddress &address, addresses) {
            recipients << address.address();
        }
        return recipients;
    }
    else if (role == MessageRecipientsDisplayNameRole) {
        QMailMessageMetaData messageMetaData(msgId);
        QStringList recipients;
        QList<QMailAddress> addresses = messageMetaData.recipients();
        foreach (const QMailAddress &address, addresses) {
            recipients << address.name();
        }
        return recipients;
    }
    else if (role == MessageReadStatusRole) {
        QMailMessageMetaData messageMetaData(msgId);

        if (messageMetaData.status() & QMailMessage::Read)
            return 1; // 1 for read
        else
            return 0; // 0 for unread
    }
    else if (role == QMailMessageModelBase::MessageBodyTextRole) {
        QMailMessage message (msgId);
        return (bodyPlainText(message));
    }
    else if (role == MessageHtmlBodyRole) {
        QMailMessage message (msgId);
        return (bodyHtmlText(&message));
    }
    else if (role == MessageQuotedBodyRole) {
        QMailMessage message (msgId);
        QString body = bodyPlainText(message);
        body.prepend('\n');
        body.replace('\n', "\n>");
        body.truncate(body.size() - 1);  // remove the extra ">" put there by QString.replace
        return body;
    }
    else if (role == MessageUuidRole) {
        QString uuid = QString::number(msgId.toULongLong());
        /* What the ???
        QMailMessage newId = QMailMessageId (uuid.toULongLong());
        QMailMessage message (newId);
        QString body = message.body().data();
        */
        return uuid;
    }
    else if (role == MessageSenderDisplayNameRole) {
        QMailMessageMetaData messageMetaData(msgId);
        return messageMetaData.from().name();
    }
    else if (role == MessageSenderEmailAddressRole) {
        QMailMessageMetaData messageMetaData(msgId);
        return messageMetaData.from().address();
    }
    else if (role == MessageCcRole) {
        QMailMessage message (msgId);
        return QMailAddress::toStringList (message.cc());
    }
    else if (role == MessageBccRole) {
        QMailMessage message (msgId);
        return QMailAddress::toStringList (message.bcc());
    }
    else if (role == MessageTimeStampRole) {
        QMailMessageMetaData messageMetaData(msgId);
        return (messageMetaData.date().toLocalTime());
    }
    else if (role == MessageSelectModeRole) {
       int selected = 0;
       if (m_selectedMsgIds.contains(msgId) == true)
           selected = 1;
        return (selected);
    }

    return QMailMessageListModel::data(index, role);
}

void EmailMessageListModel::setSearch(const QString search)
{

    if(search.isEmpty()) {
        setKey(QMailMessageKey::nonMatchingKey());
    } else {
        if(m_search == search)
            return;
        QMailMessageKey subjectKey = QMailMessageKey::subject(search, QMailDataComparator::Includes);
        QMailMessageKey toKey = QMailMessageKey::recipients(search, QMailDataComparator::Includes);
        QMailMessageKey fromKey = QMailMessageKey::sender(search, QMailDataComparator::Includes);
        setKey(m_key & (subjectKey | toKey | fromKey));
    }
    m_search = search;
}

void EmailMessageListModel::setFolderKey (QVariant id)
{
    m_currentFolderId = id.value<QMailFolderId>();
    if (!m_currentFolderId.isValid())
        return;
    QMailMessageKey folderKey = QMailMessageKey::parentFolderId(m_currentFolderId);
    QMailMessageListModel::setKey(folderKey);
    m_key=key();
    QMailMessageSortKey sortKey = QMailMessageSortKey::timeStamp(Qt::DescendingOrder);
    QMailMessageListModel::setSortKey(sortKey);
}

void EmailMessageListModel::setAccountKey (QVariant id)
{
    QMailAccountId accountId = id.value<QMailAccountId>();
    QMailAccountIdList ids;
    if (!accountId.isValid() || id == -1) {
        ids = QMailStore::instance()->queryAccounts(
                QMailAccountKey::status(QMailAccount::Enabled, QMailDataComparator::Includes),
                QMailAccountSortKey::name());
    }
    else {
        ids << accountId;
    }

    QMailFolderIdList folderIdList;

    for (int i = 0; i < ids.size(); i++) {
        QMailFolderKey key = QMailFolderKey::parentAccountId(accountId);
        QMailFolderIdList  mailFolderIds = QMailStore::instance()->queryFolders(key);
        foreach (const QMailFolderId &folderId, mailFolderIds) {
            QMailFolder folder(folderId);
            if (QString::compare(folder.displayName(), "INBOX", Qt::CaseInsensitive) == 0) {
                folderIdList << folderId;
                break;
            }
        }
    }

    QMailMessageKey accountKey = QMailMessageKey::parentAccountId(ids);
    QMailMessageListModel::setKey(accountKey);

    if(folderIdList.count() != 0) {
        // default to INBOX for now
        QMailMessageKey folderKey = QMailMessageKey::parentFolderId(folderIdList);
        QMailMessageListModel::setKey(folderKey);//!FIXME: should this be folderKey&accountKey?
    } else {
        connect(QMailStore::instance(), SIGNAL(foldersAdded ( const QMailFolderIdList &)), this,
                SLOT(foldersAdded( const QMailFolderIdList &)));
    }

    QMailMessageSortKey sortKey = QMailMessageSortKey::timeStamp(Qt::DescendingOrder);
    QMailMessageListModel::setSortKey(sortKey);

    m_key= key();
    
}

void EmailMessageListModel::foldersAdded(const QMailFolderIdList &folderIds)
{
    QMailFolderIdList folderIdList;
    foreach (const QMailFolderId &folderId, folderIds) {
        QMailFolder folder(folderId);
        if (QString::compare(folder.displayName(), "INBOX", Qt::CaseInsensitive) == 0) {
            folderIdList << folderId;
            break;
        }
    }
    if(folderIdList.count() != 0) {
        // default to INBOX for now
        QMailMessageKey folderKey = QMailMessageKey::parentFolderId(folderIdList);
        QMailMessageListModel::setKey(folderKey);//!FIXME: should this be folderKey&accountKey?
        disconnect(QMailStore::instance(), SIGNAL(foldersAdded ( const QMailFolderIdList &)), this,
                   SLOT(foldersAdded( const QMailFolderIdList &)));
        m_key = key();
    }
}

void EmailMessageListModel::sortBySender(int key)
{
    impl()->reset();
    QMailMessageSortKey sortKey;
    if (key == 0)  // descending
        sortKey = QMailMessageSortKey::sender(Qt::DescendingOrder);
    else
        sortKey = QMailMessageSortKey::sender(Qt::AscendingOrder);

    QMailMessageListModel::setSortKey(sortKey);
}

void EmailMessageListModel::sortBySubject(int key)
{
    QMailMessageSortKey sortKey;
    if (key == 0)  // descending
        sortKey = QMailMessageSortKey::subject(Qt::DescendingOrder);
    else
        sortKey = QMailMessageSortKey::subject(Qt::AscendingOrder);

    QMailMessageListModel::setSortKey(sortKey);
}

void EmailMessageListModel::sortByDate(int key)
{
    QMailMessageSortKey sortKey;
    if (key == 0)  // descending
        sortKey = QMailMessageSortKey::timeStamp(Qt::DescendingOrder);
    else
        sortKey = QMailMessageSortKey::timeStamp(Qt::AscendingOrder);

    QMailMessageListModel::setSortKey(sortKey);
}

void EmailMessageListModel::sortByAttachment(int key)
{
    // TDB
    Q_UNUSED(key);
}

void EmailMessageListModel::initMailServer()
{
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

QVariant EmailMessageListModel::indexFromMessageId (QString uuid)
{
    quint64 id = uuid.toULongLong();
    QMailMessageId msgId (id);
    for (int row = 0; row < rowCount(); row++) {
        QVariant vMsgId = data(index(row), QMailMessageModelBase::MessageIdRole);
        
        if (msgId == vMsgId.value<QMailMessageId>())
            return row;
    }
    return -1;
}

QVariant EmailMessageListModel::messageId (int idx)
{
    QMailMessageId id = idFromIndex (index(idx));
    return id;
}

QVariant EmailMessageListModel::subject (int idx)
{
    return data(index(idx), QMailMessageModelBase::MessageSubjectTextRole);
}

QVariant EmailMessageListModel::mailSender (int idx)
{
    return data(index(idx), QMailMessageModelBase::MessageAddressTextRole);
}

QVariant EmailMessageListModel::timeStamp (int idx)
{
    return data(index(idx), QMailMessageModelBase::MessageTimeStampTextRole);
}

QVariant EmailMessageListModel::body (int idx)
{
    return data(index(idx), QMailMessageModelBase::MessageBodyTextRole);
}

QVariant EmailMessageListModel::quotedBody (int idx)
{
    return data(index(idx), MessageQuotedBodyRole);
}

QVariant EmailMessageListModel::htmlBody (int idx)
{
    return data(index(idx), MessageHtmlBodyRole);
}

QVariant EmailMessageListModel::attachments (int idx)
{
    return data(index(idx), MessageAttachmentsRole);
}

QVariant EmailMessageListModel::numberOfAttachments (int idx)
{
    return data(index(idx), MessageAttachmentCountRole);
}

QVariant EmailMessageListModel::toList (int idx)
{
    return data(index(idx), MessageRecipientsRole);
}

QVariant EmailMessageListModel::recipients (int idx)
{
    QMailMessageId msgId = idFromIndex(index(idx));
    QMailMessageMetaData messageMetaData(msgId);
    QStringList recipients;
     
    QMailAccount mailAccount (messageMetaData.parentAccountId());
    QString myEmailAddress = mailAccount.fromAddress().address();

    // Since 1468833f, QMMMD::recipients() returns To: CC: and BCC: recipients
    foreach (const QMailAddress &address, messageMetaData.recipients()) {
        QString emailAddress = address.address();
        if (QString::compare(myEmailAddress, emailAddress, Qt::CaseInsensitive) != 0)
            recipients << address.toString();
    }
    return recipients;
}

QVariant EmailMessageListModel::ccList (int idx)
{
    return data(index(idx), MessageCcRole);
}

QVariant EmailMessageListModel::bccList (int idx)
{
    return data(index(idx), MessageBccRole);
}

QVariant EmailMessageListModel::messageRead (int idx)
{
    return data(index(idx), MessageReadStatusRole);
}

int EmailMessageListModel::messagesCount ()
{
    return rowCount();
}

void EmailMessageListModel::deSelectAllMessages()
{
    if (m_selectedMsgIds.size() == 0)
        return;

    QMailMessageIdList msgIds = m_selectedMsgIds;
    m_selectedMsgIds.clear();
    foreach (const QMailMessageId &msgId,  msgIds) {
        for (int row = 0; row < rowCount(); row++) {
            QVariant vMsgId = data(index(row), QMailMessageModelBase::MessageIdRole);
    
            if (msgId == vMsgId.value<QMailMessageId>())
                dataChanged (index(row), index(row));
        }
    }
}

void EmailMessageListModel::selectMessage( int idx )
{
    QMailMessageId msgId = idFromIndex(index(idx));

    if (!m_selectedMsgIds.contains (msgId)) {
        m_selectedMsgIds.append(msgId);
        dataChanged(index(idx), index(idx));
    }
}

void EmailMessageListModel::deSelectMessage (int idx )
{
    QMailMessageId msgId = idFromIndex(index(idx));

    m_selectedMsgIds.removeOne(msgId);
    dataChanged(index(idx), index(idx));
}

void EmailMessageListModel::moveSelectedMessageIds(QVariant vFolderId)
{
    if (m_selectedMsgIds.empty())
        return;

    QMailFolderId const id(vFolderId.value<QMailFolderId>());

    QMailMessage const msg(m_selectedMsgIds[0]);

    m_storageAction->onlineMoveMessages(m_selectedMsgIds, id);
    m_selectedMsgIds.clear();
    m_retrievalAction->exportUpdates(msg.parentAccountId());
}


void EmailMessageListModel::deleteSelectedMessageIds()
{
    if (m_selectedMsgIds.empty())
        return;

    QMailMessage const msg(m_selectedMsgIds[0]);

    m_storageAction->deleteMessages(m_selectedMsgIds);
    m_selectedMsgIds.clear();
    m_retrievalAction->exportUpdates(msg.parentAccountId());
}

void EmailMessageListModel::downloadActivityChanged(QMailServiceAction::Activity activity)
{
    if (QMailServiceAction *action = static_cast<QMailServiceAction*>(sender())) {
        if (activity == QMailServiceAction::Successful) {
            if (action == m_retrievalAction) {
                emit messageDownloadCompleted();
            }
        }
        else if (activity == QMailServiceAction::Failed) {
            //  Todo:  hmm.. may be I should emit an error here.
            emit messageDownloadCompleted();
        }
    }
}
