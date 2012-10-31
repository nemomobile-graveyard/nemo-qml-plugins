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

    void sendMessages (const QMailAccountId &id);
    bool isSynchronizing() const;

    void exportAccountChanges(const QMailAccountId id);
    void flagMessages(const QMailMessageIdList &ids, quint64 setMask, quint64 unsetMask);

    Q_INVOKABLE void accountsSync();
    Q_INVOKABLE void deleteMessage(QVariant id);
    Q_INVOKABLE void deleteMessages(const QMailMessageIdList &ids);
    Q_INVOKABLE void createFolder (const QString &name, QVariant vMailAccountId, QVariant vParentFolderId);
    Q_INVOKABLE void deleteFolder(QVariant vFolderId);
    Q_INVOKABLE void renameFolder(QVariant vFolderId, const QString &name);
    Q_INVOKABLE void synchronize (QVariant vMailAccountId);
    Q_INVOKABLE void cancelSync ();
    Q_INVOKABLE void markMessageAsRead (QVariant vMsgId);
    Q_INVOKABLE void markMessageAsUnread (QVariant vMsgId);
    Q_INVOKABLE void getMoreMessages (QVariant vFolderId);
    Q_INVOKABLE QString getSignatureForAccount (QVariant vMailAccountId);
    Q_INVOKABLE bool confirmDeleteMail ();
    Q_INVOKABLE void downloadAttachment(QVariant vMailMessage, const QString& attachmentDisplayName);
    Q_INVOKABLE bool openAttachment(const QString& attachmentDisplayName);
    Q_INVOKABLE void openBrowser(const QString& url);
    Q_INVOKABLE QString getMessageBodyFromFile(const QString& bodyFilePath);


signals:
    void retrievalCompleted();
    void sendCompleted();
    void syncCompleted();
    void syncBegin();
    void messagesDeleted(QMailMessageIdList);
    void error(const QMailAccountId &id, const QString &msg, int code);
    void attachmentDownloadStarted();
    void attachmentDownloadCompleted();
    void progressUpdate (int percent);

private slots:
    void progressChanged(uint value, uint total);
    void activityChanged(QMailServiceAction::Activity activity);
    void attachmentDownloadActivityChanged(QMailServiceAction::Activity activity);
    void exportActivityChanged(QMailServiceAction::Activity activity);
    void exportAccounts();
    void initMailServer();
    void onFoldersAdded (const QMailFolderIdList &);

private:
    static EmailAgent *m_instance;

    bool m_retrieving;
    bool m_transmitting;
    bool m_exporting;
    bool m_cancelling;

    QMailAccountIdList m_retrieveAccounts;
    QMailAccountIdList m_transmitAccounts;
    QMailAccountIdList m_exportAccounts;

    QMailRetrievalAction *const m_retrievalAction;
    QMailStorageAction *const m_storageAction;
    QMailTransmitAction *const m_transmitAction;
    QMailRetrievalAction *const m_exportAction;
    QMailRetrievalAction *m_attachmentRetrievalAction;
    QMailMessageId m_messageId;
    QMailMessagePart m_attachmentPart;

    QProcess m_messageServerProcess;

    QTimer m_exportTimer;

#ifdef HAS_MLITE
    MGConfItem *m_confirmDeleteMail;
#endif
};

#endif
