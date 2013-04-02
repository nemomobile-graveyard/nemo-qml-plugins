/*
 * Copyright 2011 Intel Corporation.
 * Copyright (C) 2012 Jolla Ltd.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 	
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef EMAILMESSAGELISTMODEL_H
#define EMAILMESSAGELISTMODEL_H

#include <QAbstractListModel>
#include <QProcess>

#include <qmailmessage.h>
#include <qmailmessagelistmodel.h>
#include <qmailserviceaction.h>
#include <qmailaccount.h>


class EmailMessageListModel : public QMailMessageListModel
{
    Q_OBJECT
    Q_ENUMS(Priority)
    Q_ENUMS(Sort)

public:
    enum Roles
    {
        MessageAttachmentCountRole = QMailMessageModelBase::MessageIdRole + 1, // returns number of attachment
        MessageAttachmentsRole,                                // returns a list of attachments
        MessageRecipientsRole,                                 // returns a list of recipients (email address)
        MessageRecipientsDisplayNameRole,                      // returns a list of recipients (displayName)
        MessageReadStatusRole,                                 // returns the read/unread status
        MessageQuotedBodyRole,                                 // returns the quoted body
        MessageHtmlBodyRole,                                   // returns the html body
        MessageUuidRole,			                           // returns a unique string id
        MessageSenderDisplayNameRole,                          // returns sender's display name
        MessageSenderEmailAddressRole,                         // returns sender's email address
        MessageToRole,                                         // returns a list of To (email + displayName)
        MessageCcRole,                                         // returns a list of Cc (email + displayName)
        MessageBccRole,                                        // returns a list of Bcc (email + displayName)
        MessageTimeStampRole,                                  // returns timestamp in QDateTime format
        MessageSelectModeRole,                                 // returns the select mode
        MessagePreviewRole,                                    // returns message preview if available
        MessageTimeSectionRole,                                // returns time section relative to the current time
        MessagePriorityRole,                                   // returns message priority
        MessageAccountIdRole,                                  // returns parent account id for the message
        MessageHasAttachmentsRole,                             // returns 1 if message has attachments, 0 otherwise
        MessageSizeSectionRole                                 // returns size section (0-2)
    };

    EmailMessageListModel(QObject *parent = 0);
    ~EmailMessageListModel();
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QString bodyHtmlText(const QMailMessage &) const;
    QString bodyPlainText(const QMailMessage &) const;

    enum Priority { LowPriority, NormalPriority, HighPriority };

    enum Sort { Time, Sender, Size, ReadStatus, Priority, Attachments, Subject};

    Q_PROPERTY(bool combinedInbox READ combinedInbox WRITE setCombinedInbox NOTIFY combinedInboxChanged)
    Q_PROPERTY(bool filterUnread READ filterUnread WRITE setFilterUnread NOTIFY filterUnreadChanged)

    // property accessors.
    bool combinedInbox() const;
    void setCombinedInbox(bool c);
    bool filterUnread() const;
    void setFilterUnread(bool u);

Q_SIGNALS:
    void combinedInboxChanged();
    void filterUnreadChanged();

signals:
    void messageDownloadCompleted();

public slots:
    Q_INVOKABLE void setFolderKey(QVariant id);
    Q_INVOKABLE void setAccountKey(QVariant id);
    Q_INVOKABLE void sortBySender(int order = 0);
    Q_INVOKABLE void sortBySubject(int order = 0);
    Q_INVOKABLE void sortByDate(int order = 1);
    Q_INVOKABLE void sortByAttachment(int order = 1);
    Q_INVOKABLE void sortByReadStatus(int order = 0);
    Q_INVOKABLE void sortByPriority(int order = 1);
    Q_INVOKABLE void sortBySize(int order = 1);
    Q_INVOKABLE void setSearch(const QString search);

    Q_INVOKABLE QVariant accountIdForMessage(QVariant messageId);
    Q_INVOKABLE QVariant indexFromMessageId(QString msgId);
    Q_INVOKABLE QVariant folderIdForMessage(QVariant messageId);
    Q_INVOKABLE QVariant messageId(int index);
    Q_INVOKABLE QVariant subject(int index);
    Q_INVOKABLE QVariant mailSender(int index);
    Q_INVOKABLE QVariant senderDisplayName(int index);
    Q_INVOKABLE QVariant senderEmailAddress(int index);
    Q_INVOKABLE QVariant timeStamp(int index);
    Q_INVOKABLE QVariant body(int index);
    Q_INVOKABLE QVariant htmlBody(int index);
    Q_INVOKABLE QVariant quotedBody(int index);
    Q_INVOKABLE QVariant attachments(int index);
    Q_INVOKABLE QVariant numberOfAttachments(int index);
    Q_INVOKABLE QVariant recipients(int index);
    Q_INVOKABLE QVariant ccList(int index);
    Q_INVOKABLE QVariant bccList(int index);
    Q_INVOKABLE QVariant toList(int index);
    Q_INVOKABLE QVariant messageRead(int index);
    Q_INVOKABLE QVariant size(int index);
    Q_INVOKABLE QVariant accountId(int index);
    Q_INVOKABLE QVariant priority(int index);
    Q_INVOKABLE int messagesCount();
    Q_INVOKABLE void deSelectAllMessages();
    Q_INVOKABLE void selectMessage(int index);
    Q_INVOKABLE void deSelectMessage(int index);
    Q_INVOKABLE void moveSelectedMessageIds(QVariant vFolderId);
    Q_INVOKABLE void deleteSelectedMessageIds();
    Q_INVOKABLE void markAllMessagesAsRead();

    void foldersAdded(const QMailFolderIdList &folderIds);

private slots:
    void downloadActivityChanged(QMailServiceAction::Activity);

private:

    bool m_combinedInbox;
    bool m_filterUnread;
    QProcess m_msgAccount;
    QMailFolderId m_currentFolderId;
    QProcess m_messageServerProcess;
    QMailAccountIdList m_mailAccountIds;
    QMailRetrievalAction *m_retrievalAction;
    QString m_search;
    QMailMessageKey m_key;                  // key set externally other than search
    QMailMessageSortKey m_sortKey;
    QList<QMailMessageId> m_selectedMsgIds;
};

#endif
