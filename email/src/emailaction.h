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
    EmailAction(QMailServiceAction* action, bool onlineAction = true);

public:
    virtual ~EmailAction();
    virtual void execute() = 0;
    //virtual QMailServiceAction* serviceAction() const = 0;
    bool operator==(const EmailAction &action) const;
    QString description() const;

    QMailServiceAction *serviceAction;


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
    DeleteMessages(QMailServiceAction* action, const QMailMessageIdList &ids);
    ~DeleteMessages();
    void execute();

private:
    QMailMessageIdList _ids;
};

class ExportUpdates : public EmailAction
{
public:
    ExportUpdates(QMailServiceAction* action, const QMailAccountId& id);
    ~ExportUpdates();
    void execute();

private:
    QMailAccountId _accountId;
};

class FlagMessages : public EmailAction
{
public:
    FlagMessages(QMailServiceAction* action, const QMailMessageIdList & ids,
                 quint64 setMask, quint64 unsetMask);
    ~FlagMessages();
    void execute();

private:
    QMailMessageIdList _ids;
    quint64 _setMask;
    quint64 _unsetMask;
};

class OnlineCreateFolder : public EmailAction
{
public:
    OnlineCreateFolder(QMailServiceAction* action, const QString &name,
                       const QMailAccountId& id, const QMailFolderId &parentId);
    ~OnlineCreateFolder();
    void execute();

private:
    QString _name;
    QMailAccountId _accountId;
    QMailFolderId _parentId;
};

class OnlineDeleteFolder : public EmailAction
{
public:
    OnlineDeleteFolder(QMailServiceAction* action, const QMailFolderId &folderId);
    ~OnlineDeleteFolder();
    void execute();

private:
    QMailFolderId _folderId;
};

class OnlineRenameFolder : public EmailAction
{
public:
    OnlineRenameFolder(QMailServiceAction* action, const QMailFolderId &folderId, const QString &name);
    ~OnlineRenameFolder();
    void execute();

private:
    QMailFolderId _folderId;
    QString _name;
};

class RetrieveMessageList : public EmailAction
{
public:
    RetrieveMessageList(QMailServiceAction* action, const QMailAccountId& id,
                        const QMailFolderId &folderId, uint minimum);
    ~RetrieveMessageList();
    void execute();

private:
    QMailAccountId _accountId;
    QMailFolderId _folderId;
    uint _minimum;
};

class RetrieveFolderList : public EmailAction
{
public:
    RetrieveFolderList(QMailServiceAction* action, const QMailAccountId& id,
                        const QMailFolderId &folderId, uint descending);
    ~RetrieveFolderList();
    void execute();

private:
    QMailAccountId _accountId;
    QMailFolderId _folderId;
    uint _descending;
};

class Synchronize : public EmailAction
{
public:
    Synchronize(QMailServiceAction* action, const QMailAccountId& id);
    ~Synchronize();
    void execute();

private:
    QMailAccountId _accountId;
};

class TransmitMessages : public EmailAction
{
public:
    TransmitMessages(QMailServiceAction* action, const QMailAccountId& id);
    ~TransmitMessages();
    void execute();

private:
    QMailAccountId _accountId;
};

#endif // EMAILACTION_H
