/*
 * Copyright (C) 2013 Jolla Ltd.
 * Contact: Valerio Valerio <valerio.valerio@jollamobile.com>
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef EMAILACTION_H
#define EMAILACTION_H

#include <QObject>
#include <qmailserviceaction.h>

class EmailAction : public QObject
{
     Q_OBJECT

protected:
    EmailAction( bool onlineAction = true);

public:
    virtual ~EmailAction();
    virtual void execute() = 0;
    virtual QMailServiceAction* serviceAction() const = 0;
    bool operator==(const EmailAction &action) const;
    QString description() const;

    quint64 id;

protected:
    QString _description;

private:
    bool _onlineAction;
    bool needsNetworkConnection() const { return _onlineAction; }
};

class DeleteMessages : public EmailAction
{
public:
    DeleteMessages(QMailStorageAction* storageAction, const QMailMessageIdList &ids);
    ~DeleteMessages();
    void execute();
    QMailServiceAction* serviceAction() const;

private:
    QMailStorageAction* _storageAction;
    QMailMessageIdList _ids;
};

class ExportUpdates : public EmailAction
{
public:
    ExportUpdates(QMailRetrievalAction* retrievalAction, const QMailAccountId& id);
    ~ExportUpdates();
    void execute();
    QMailServiceAction* serviceAction() const;

private:
    QMailRetrievalAction* _retrievalAction;
    QMailAccountId _accountId;
};

class FlagMessages : public EmailAction
{
public:
    FlagMessages(QMailStorageAction* storageAction, const QMailMessageIdList & ids,
                 quint64 setMask, quint64 unsetMask);
    ~FlagMessages();
    void execute();
    QMailServiceAction* serviceAction() const;

private:
    QMailStorageAction* _storageAction;
    QMailMessageIdList _ids;
    quint64 _setMask;
    quint64 _unsetMask;
};

class OnlineCreateFolder : public EmailAction
{
public:
    OnlineCreateFolder(QMailStorageAction *storageAction, const QString &name,
                       const QMailAccountId& id, const QMailFolderId &parentId);
    ~OnlineCreateFolder();
    void execute();
    QMailServiceAction* serviceAction() const;

private:
    QMailStorageAction* _storageAction;
    QString _name;
    QMailAccountId _accountId;
    QMailFolderId _parentId;
};

class OnlineDeleteFolder : public EmailAction
{
public:
    OnlineDeleteFolder(QMailStorageAction *storageAction, const QMailFolderId &folderId);
    ~OnlineDeleteFolder();
    void execute();
    QMailServiceAction* serviceAction() const;

private:
    QMailStorageAction* _storageAction;
    QMailFolderId _folderId;
};

class OnlineRenameFolder : public EmailAction
{
public:
    OnlineRenameFolder(QMailStorageAction *storageAction, const QMailFolderId &folderId, const QString &name);
    ~OnlineRenameFolder();
    void execute();
    QMailServiceAction* serviceAction() const;

private:
    QMailStorageAction* _storageAction;
    QMailFolderId _folderId;
    QString _name;
};

class RetrieveMessageList : public EmailAction
{
public:
    RetrieveMessageList(QMailRetrievalAction* retrievalAction, const QMailAccountId& id,
                        const QMailFolderId &folderId, uint minimum);
    ~RetrieveMessageList();
    void execute();
    QMailServiceAction* serviceAction() const;

private:
    QMailRetrievalAction* _retrievalAction;
    QMailAccountId _accountId;
    QMailFolderId _folderId;
    uint _minimum;
};

class RetrieveFolderList : public EmailAction
{
public:
    RetrieveFolderList(QMailRetrievalAction* retrievalAction, const QMailAccountId& id,
                        const QMailFolderId &folderId, uint descending);
    ~RetrieveFolderList();
    void execute();
    QMailServiceAction* serviceAction() const;

private:
    QMailRetrievalAction* _retrievalAction;
    QMailAccountId _accountId;
    QMailFolderId _folderId;
    uint _descending;
};

class Synchronize : public EmailAction
{
public:
    Synchronize(QMailRetrievalAction* retrievalAction, const QMailAccountId& id);
    ~Synchronize();
    void execute();
    QMailServiceAction* serviceAction() const;

private:
    QMailRetrievalAction* _retrievalAction;
    QMailAccountId _accountId;
};

class TransmitMessages : public EmailAction
{
public:
    TransmitMessages(QMailTransmitAction* transmitAction, const QMailAccountId& id);
    ~TransmitMessages();
    void execute();
    QMailServiceAction* serviceAction() const;

private:
    QMailTransmitAction* _transmitAction;
    QMailAccountId _accountId;
};

#endif // EMAILACTION_H
