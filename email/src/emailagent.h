/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 	
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef EMAILAGENT_H
#define EMAILAGENT_H

#include <QDeclarativeItem>
#include <QProcess>
#include <QTimer>

#include <qmailaccount.h>
#include <qmailstore.h>
#include <qmailserviceaction.h>

#include "emailaction.h"

#ifdef HAS_MLITE
#include <mgconfitem.h>
#endif

class EmailAgent : public QDeclarativeItem
{
    Q_OBJECT

public:
    static EmailAgent *instance();

    explicit EmailAgent (QDeclarativeItem *parent = 0);
    ~EmailAgent();

    void initMailServer();
    void sendMessages (const QMailAccountId &id);
    bool isSynchronizing() const;

    void exportUpdates(const QMailAccountId id);
    void flagMessages(const QMailMessageIdList &ids, quint64 setMask, quint64 unsetMask);
    void moveMessages(const QMailMessageIdList &ids, const QMailFolderId &destinationId);
    quint64 newAction();

    Q_INVOKABLE void accountsSync(const bool syncOnlyInbox = false, const uint minimum = 20);
    Q_INVOKABLE void deleteMessage(QVariant id);
    Q_INVOKABLE void deleteMessages(const QMailMessageIdList &ids);
    Q_INVOKABLE void createFolder(const QString &name, QVariant vMailAccountId, QVariant vParentFolderId);
    Q_INVOKABLE void deleteFolder(QVariant vFolderId);
    Q_INVOKABLE void renameFolder(QVariant vFolderId, const QString &name);
    Q_INVOKABLE void retrieveFolderList(QVariant vMailAccountId, QVariant vFolderId = 0, const bool descending = true);
    Q_INVOKABLE void synchronize(QVariant vMailAccountId);
    Q_INVOKABLE void synchronizeInbox(QVariant mailAccountId, const uint minimum = 20);
    Q_INVOKABLE void retrieveMessageList(QVariant vMailAccountId, QVariant vFolderId, const uint minimum = 20);
    Q_INVOKABLE void retrieveMessageRange(QVariant messageId, uint minimum);
    Q_INVOKABLE void cancelSync();
    Q_INVOKABLE void markMessageAsRead(QVariant vMsgId);
    Q_INVOKABLE void markMessageAsUnread(QVariant vMsgId);
    Q_INVOKABLE void getMoreMessages(QVariant vFolderId, uint minimum = 20);
    Q_INVOKABLE QString getSignatureForAccount(QVariant vMailAccountId);
    Q_INVOKABLE bool confirmDeleteMail();
    Q_INVOKABLE void downloadAttachment(QVariant vMailMessage, const QString& attachmentDisplayName);
    Q_INVOKABLE bool openAttachment(const QString& attachmentDisplayName);
    Q_INVOKABLE void openBrowser(const QString& url);
    Q_INVOKABLE QString getMessageBodyFromFile(const QString& bodyFilePath);
    Q_INVOKABLE QVariant inboxFolderId(QVariant vMailAccountId);

signals:
    void standardFoldersCreated(const QMailAccountId &id);
    void retrievalCompleted();
    void sendCompleted();
    void syncCompleted();
    void syncBegin();
    void error(const QMailAccountId &id, const QString &msg, int code);
    void attachmentDownloadStarted();
    void attachmentDownloadCompleted();
    void progressUpdate (int percent);

private slots:
    void progressChanged(uint value, uint total);
    void activityChanged(QMailServiceAction::Activity activity);
    void attachmentDownloadActivityChanged(QMailServiceAction::Activity activity);
    void onFoldersAdded (const QMailFolderIdList &);
    void onStandardFoldersCreated(const QMailAccountId &accountId);

private:
    static EmailAgent *m_instance;

    uint m_actionCount;
    bool m_retrieving;
    bool m_transmitting;
    bool m_cancelling;
    bool m_synchronizing;

    QMailAccountIdList m_enabledAccounts;

    QScopedPointer<QMailRetrievalAction> const m_retrievalAction;
    QScopedPointer<QMailStorageAction> const m_storageAction;
    QScopedPointer<QMailTransmitAction> const m_transmitAction;
    QMailRetrievalAction *m_attachmentRetrievalAction;
    QMailMessageId m_messageId;
    QMailMessagePart m_attachmentPart;

    QProcess m_messageServerProcess;

    void enqueue(EmailAction *action);
    void dequeue();
    bool actionInQueue(QSharedPointer<EmailAction> action) const;
    void executeCurrent();
    QSharedPointer<EmailAction> getNext();

    QList<QSharedPointer<EmailAction> > _actionQueue;
    QSharedPointer<EmailAction> _currentAction;

#ifdef HAS_MLITE
    MGConfItem *m_confirmDeleteMail;
#endif
};

#endif
