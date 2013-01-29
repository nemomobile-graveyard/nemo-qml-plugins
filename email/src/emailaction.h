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

public:
    enum ActionType {
        Export = 0,
        Retrieve,
        RetrieveFolderList,
        Send,
        StandardFolders,
        Storage,
        Transmit
    };

    virtual ~EmailAction();
    virtual void execute() = 0;
    virtual QMailAccountId accountId() const;
    virtual QMailServiceAction* serviceAction() const = 0;
    bool operator==(const EmailAction &action) const;
    QString description() const;
    ActionType type() const;
    quint64 id() const;
    void setId(const quint64 id);

protected:
    EmailAction(bool onlineAction = true);

    QString _description;
    ActionType _type;
    quint64 _id;

private:
    bool _onlineAction;
    bool needsNetworkConnection() const { return _onlineAction; }
};

class CreateStandardFolders : public EmailAction
{
public:
    CreateStandardFolders(QMailRetrievalAction* retrievalAction, const QMailAccountId& id);
    ~CreateStandardFolders();
    void execute();
    QMailServiceAction* serviceAction() const;
    QMailAccountId accountId() const;

private:
    QMailRetrievalAction* _retrievalAction;
    QMailAccountId _accountId;
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

class MoveToFolder : public EmailAction
{
public:
    MoveToFolder(QMailStorageAction *storageAction, const QMailMessageIdList &ids,
                 const QMailFolderId &folderId);
    ~MoveToFolder();
    void execute();
    QMailServiceAction* serviceAction() const;

private:
    QMailStorageAction* _storageAction;
    QMailMessageIdList _ids;
    QMailFolderId _destinationFolder;
};

class MoveToStandardFolder : public EmailAction
{
public:
    MoveToStandardFolder(QMailStorageAction *storageAction, const QMailMessageIdList &ids,
                         QMailFolder::StandardFolder standardFolder);
    ~MoveToStandardFolder();
    void execute();
    QMailServiceAction* serviceAction() const;

private:
    QMailStorageAction* _storageAction;
    QMailMessageIdList _ids;
    QMailFolder::StandardFolder _standardFolder;
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

class OnlineMoveMessages : public EmailAction
{
public:
    OnlineMoveMessages(QMailStorageAction *storageAction, const QMailMessageIdList &ids,
                       const QMailFolderId &destinationId);
    ~OnlineMoveMessages();
    void execute();
    QMailServiceAction* serviceAction() const;

private:
    QMailStorageAction* _storageAction;
    QMailMessageIdList _ids;
    QMailFolderId _destinationId;
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

class RetrieveFolderList : public EmailAction
{
public:
    RetrieveFolderList(QMailRetrievalAction* retrievalAction, const QMailAccountId& id,
                        const QMailFolderId &folderId, uint descending = true);
    ~RetrieveFolderList();
    void execute();
    QMailServiceAction* serviceAction() const;
    QMailAccountId accountId() const;

private:
    QMailRetrievalAction* _retrievalAction;
    QMailAccountId _accountId;
    QMailFolderId _folderId;
    uint _descending;
};

class RetrieveMessageList : public EmailAction
{
public:
    RetrieveMessageList(QMailRetrievalAction* retrievalAction, const QMailAccountId& id,
                        const QMailFolderId &folderId, uint minimum,
                        const QMailMessageSortKey &sort = QMailMessageSortKey());
    ~RetrieveMessageList();
    void execute();
    QMailServiceAction* serviceAction() const;

private:
    QMailRetrievalAction* _retrievalAction;
    QMailAccountId _accountId;
    QMailFolderId _folderId;
    uint _minimum;
    QMailMessageSortKey _sort;
};

class RetrieveMessageLists : public EmailAction
{
public:
    RetrieveMessageLists(QMailRetrievalAction* retrievalAction, const QMailAccountId& id,
                        const QMailFolderIdList & folderIds, uint minimum,
                        const QMailMessageSortKey &sort = QMailMessageSortKey());
    ~RetrieveMessageLists();
    void execute();
    QMailServiceAction* serviceAction() const;

private:
    QMailRetrievalAction* _retrievalAction;
    QMailAccountId _accountId;
    QMailFolderIdList _folderIds;
    uint _minimum;
    QMailMessageSortKey _sort;
};

class RetrieveMessagePart : public EmailAction
{
public:
    RetrieveMessagePart(QMailRetrievalAction* retrievalAction,
                        const QMailMessagePart::Location &partLocation);
    ~RetrieveMessagePart();
    void execute();
    QMailServiceAction* serviceAction() const;

private:
    QMailRetrievalAction* _retrievalAction;
    QMailMessagePart::Location _partLocation;
};

class RetrieveMessagePartRange : public EmailAction
{
public:
    RetrieveMessagePartRange(QMailRetrievalAction* retrievalAction,
                        const QMailMessagePart::Location &partLocation, uint minimum);
    ~RetrieveMessagePartRange();
    void execute();
    QMailServiceAction* serviceAction() const;

private:
    QMailRetrievalAction* _retrievalAction;
    QMailMessagePart::Location _partLocation;
    uint _minimum;
};

class RetrieveMessageRange : public EmailAction
{
public:
    RetrieveMessageRange(QMailRetrievalAction* retrievalAction,
                         const QMailMessageId &messageId, uint minimum);
    ~RetrieveMessageRange();
    void execute();
    QMailServiceAction* serviceAction() const;

private:
    QMailRetrievalAction* _retrievalAction;
    QMailMessageId _messageId;
    uint _minimum;
};

class RetrieveMessages : public EmailAction
{
public:
    RetrieveMessages(QMailRetrievalAction* retrievalAction,
                     const QMailMessageIdList & messageIds,
                     QMailRetrievalAction::RetrievalSpecification spec = QMailRetrievalAction::MetaData);
    ~RetrieveMessages();
    void execute();
    QMailServiceAction* serviceAction() const;

private:
    QMailRetrievalAction* _retrievalAction;
    QMailMessageIdList _messageIds;
    QMailRetrievalAction::RetrievalSpecification _spec;
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
    QMailAccountId accountId() const;

private:
    QMailTransmitAction* _transmitAction;
    QMailAccountId _accountId;
};

#endif // EMAILACTION_H
