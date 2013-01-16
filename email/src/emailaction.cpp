/*
 * Copyright (C) 2013 Jolla Ltd.
 * Contact: Valerio Valerio <valerio.valerio@jollamobile.com>
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "emailaction.h"

template<typename T>
QString idListToString(const QList<T> &ids)
{
    QString idsList;
    int idsCount = ids.count();
    int interatorPos = 0;
    foreach(const typename QList<T>::value_type &id, ids) {
        interatorPos++;
        if (interatorPos == idsCount) {
            idsList += QString("%1").arg(id.toULongLong());
        }
        else {
            idsList += QString("%1,").arg(id.toULongLong());
        }
    }
    return idsList;

}

/*
  EmailAction
*/
EmailAction::EmailAction(bool onlineAction)
    : _onlineAction(onlineAction)
{
}

EmailAction::~EmailAction()
{
}

bool EmailAction::operator==(const EmailAction &action) const
{
    if(action._description.isEmpty() || _description.isEmpty()) {
        return false;
    }
    if(action._description == _description) {
        return true;
    }
    else {
        return false;
    }
}

QString EmailAction::description() const
{
    return _description;
}

/*
  DeleteMessages
*/

DeleteMessages::DeleteMessages(QMailStorageAction* storageAction, const QMailMessageIdList &ids)
    : EmailAction()
    , _storageAction(storageAction)
    , _ids(ids)
{
    QString idsList = idListToString(_ids);
    _description = QString("delete-messages:message-ids=%1").arg(idsList);
}

DeleteMessages::~DeleteMessages()
{
}

void DeleteMessages::execute()
{
    _storageAction->deleteMessages(_ids);
}

QMailServiceAction* DeleteMessages::serviceAction() const
{
    return _storageAction;
}

/*
  ExportUpdates
*/
ExportUpdates::ExportUpdates(QMailRetrievalAction *retrievalAction, const QMailAccountId &id)
    : EmailAction()
    , _retrievalAction(retrievalAction)
    , _accountId(id)
{
    _description = QString("exporting-updates:account-id=%1").arg(_accountId.toULongLong());
}

ExportUpdates::~ExportUpdates()
{
}

void ExportUpdates::execute()
{
    _retrievalAction->exportUpdates(_accountId);
}

QMailServiceAction* ExportUpdates::serviceAction() const
{
    return _retrievalAction;
}

/*
  FlagMessages
*/
FlagMessages::FlagMessages(QMailStorageAction* storageAction, const QMailMessageIdList &ids,
                           quint64 setMask, quint64 unsetMask)
    : EmailAction()
    , _storageAction(storageAction)
    , _ids(ids)
    , _setMask(setMask)
    , _unsetMask(unsetMask)
{
    QString idsList = idListToString(_ids);
    _description =  QString("flag-messages:message-ids=%1;setMark=%2;unsetMark=%3").arg(idsList)
            .arg(_setMask).arg(_unsetMask);
}

FlagMessages::~FlagMessages()
{
}

void FlagMessages::execute()
{
    _storageAction->flagMessages(_ids, _setMask, _unsetMask);
}

QMailServiceAction* FlagMessages::serviceAction() const
{
    return _storageAction;
}

/*
  MoveToStandardFolder
*/
MoveToStandardFolder::MoveToStandardFolder(QMailStorageAction *storageAction,
                                           const QMailMessageIdList &ids, QMailFolder::StandardFolder standardFolder)
    : EmailAction()
    , _storageAction(storageAction)
    , _ids(ids)
    , _standardFolder(standardFolder)
{
    QString idsList = idListToString(_ids);
    _description =  QString("move-messages-to-standard-folder:message-ids=%1;standard-folder=%2").arg(idsList)
            .arg(_standardFolder);
}

MoveToStandardFolder::~MoveToStandardFolder()
{
}

void MoveToStandardFolder::execute()
{
    _storageAction->moveToStandardFolder(_ids, _standardFolder);
}

QMailServiceAction* MoveToStandardFolder::serviceAction() const
{
    return _storageAction;
}

/*
  OnlineCreateFolder
*/
OnlineCreateFolder::OnlineCreateFolder(QMailStorageAction* storageAction, const QString &name
                                       , const QMailAccountId &id, const QMailFolderId &parentId)
    : EmailAction()
    , _storageAction(storageAction)
    , _name(name)
    , _accountId(id)
    , _parentId(parentId)
{
    QString pId;
    if (_parentId.isValid()) {
        pId = _parentId.toULongLong();
    }
    else {
        pId = "NULL";
    }
    _description = QString("create-folder:name=%1;account-id=%2;parent-id=%3").arg(_accountId.toULongLong())
            .arg(_name).arg(pId);
}

OnlineCreateFolder::~OnlineCreateFolder()
{
}

void OnlineCreateFolder::execute()
{
    _storageAction->onlineCreateFolder(_name, _accountId, _parentId);
}

QMailServiceAction* OnlineCreateFolder::serviceAction() const
{
    return _storageAction;
}

/*
  OnlineDeleteFolder
*/
OnlineDeleteFolder::OnlineDeleteFolder(QMailStorageAction* storageAction, const QMailFolderId &folderId)
    : EmailAction()
    , _storageAction(storageAction)
    , _folderId(folderId)
{
    _description = QString("delete-folder:folder-id=%1").arg(_folderId.toULongLong());
}

OnlineDeleteFolder::~OnlineDeleteFolder()
{
}

void OnlineDeleteFolder::execute()
{
    _storageAction->onlineDeleteFolder(_folderId);
}

QMailServiceAction* OnlineDeleteFolder::serviceAction() const
{
    return _storageAction;
}

/*
  OnlineMoveMessages
*/
OnlineMoveMessages::OnlineMoveMessages(QMailStorageAction *storageAction, const QMailMessageIdList &ids,
                                      const QMailFolderId &destinationId)
    : EmailAction()
    , _storageAction(storageAction)
    , _ids(ids)
    , _destinationId(destinationId)
{
    QString idsList = idListToString(_ids);
    _description = QString("move-messages:message-ids=%1;destination-folder=%2").arg(idsList)
            .arg(_destinationId.toULongLong());
}

OnlineMoveMessages::~OnlineMoveMessages()
{
}

void OnlineMoveMessages::execute()
{
    _storageAction->onlineMoveMessages(_ids,_destinationId);
}

QMailServiceAction* OnlineMoveMessages::serviceAction() const
{
    return _storageAction;
}

/*
  OnlineRenameFolder
*/
OnlineRenameFolder::OnlineRenameFolder(QMailStorageAction* storageAction, const QMailFolderId &folderId, const QString &name)
    : EmailAction()
    , _storageAction(storageAction)
    , _folderId(folderId)
    , _name(name)
{
    _description = QString("rename-folder:folder-id=%1;new-name=%2").arg(_folderId.toULongLong()).arg(_name);
}

OnlineRenameFolder::~OnlineRenameFolder()
{
}

void OnlineRenameFolder::execute()
{
    _storageAction->onlineRenameFolder(_folderId, _name);
}

QMailServiceAction* OnlineRenameFolder::serviceAction() const
{
    return _storageAction;
}

/*
  RetrieveFolderList
*/
RetrieveFolderList::RetrieveFolderList(QMailRetrievalAction* retrievalAction, const QMailAccountId& id,
                                       const QMailFolderId &folderId, uint descending )
    : EmailAction()
    , _retrievalAction(retrievalAction)
    , _accountId(id)
    , _folderId(folderId)
    , _descending(descending)
{
    QString fId;
    if (_folderId.isValid()) {
        fId = _folderId.toULongLong();
    }
    else {
        fId = "NULL";
    }
    _description = QString("retrieve-folder-list:account-id=%1;folder-id=%2")
            .arg(_accountId.toULongLong())
            .arg(fId);
}

RetrieveFolderList::~RetrieveFolderList()
{
}

void RetrieveFolderList::execute()
{
    _retrievalAction->retrieveFolderList(_accountId, _folderId, _descending);
}

QMailServiceAction* RetrieveFolderList::serviceAction() const
{
    return _retrievalAction;
}

/*
  RetrieveMessageList
*/
RetrieveMessageList::RetrieveMessageList(QMailRetrievalAction* retrievalAction, const QMailAccountId& id,
                    const QMailFolderId &folderId, uint minimum, const QMailMessageSortKey &sort)
    : EmailAction()
    , _retrievalAction(retrievalAction)
    , _accountId(id)
    , _folderId(folderId)
    , _minimum(minimum)
    , _sort(sort)
{
    _description = QString("retrieve-message-list:account-id=%1;folder-id=%2")
            .arg(_accountId.toULongLong())
            .arg(_folderId.toULongLong());
}

RetrieveMessageList::~RetrieveMessageList()
{
}

void RetrieveMessageList::execute()
{
    _retrievalAction->retrieveMessageList(_accountId, _folderId, _minimum, _sort);
}

QMailServiceAction* RetrieveMessageList::serviceAction() const
{
    return _retrievalAction;
}

/*
  RetrieveMessageLists
*/
RetrieveMessageLists::RetrieveMessageLists(QMailRetrievalAction* retrievalAction, const QMailAccountId& id,
                    const QMailFolderIdList &folderIds, uint minimum, const QMailMessageSortKey &sort)
    : EmailAction()
    , _retrievalAction(retrievalAction)
    , _accountId(id)
    , _folderIds(folderIds)
    , _minimum(minimum)
    , _sort(sort)
{
    QString ids = idListToString(_folderIds);
    _description = QString("retrieve-message-lists:account-id=%1;folder-ids=%2")
            .arg(_accountId.toULongLong())
            .arg(ids);
}

RetrieveMessageLists::~RetrieveMessageLists()
{
}

void RetrieveMessageLists::execute()
{
    _retrievalAction->retrieveMessageLists(_accountId, _folderIds, _minimum, _sort);
}

QMailServiceAction* RetrieveMessageLists::serviceAction() const
{
    return _retrievalAction;
}

/*
  RetrieveMessagePart
*/
RetrieveMessagePart::RetrieveMessagePart(QMailRetrievalAction *retrievalAction,
                                         const QMailMessagePartContainer::Location &partLocation)
    : EmailAction()
    , _retrievalAction(retrievalAction)
    , _partLocation(partLocation)
{
    _description = QString("retrieve-message-part:partLocation-id=%1")
            .arg(_partLocation.toString(true));
}

RetrieveMessagePart::~RetrieveMessagePart()
{
}

void RetrieveMessagePart::execute()
{
     _retrievalAction->retrieveMessagePart(_partLocation);
}

QMailServiceAction* RetrieveMessagePart::serviceAction() const
{
    return _retrievalAction;
}

/*
  RetrieveMessagePartRange
*/
RetrieveMessagePartRange::RetrieveMessagePartRange(QMailRetrievalAction *retrievalAction,
                                         const QMailMessagePartContainer::Location &partLocation, uint minimum)
    : EmailAction()
    , _retrievalAction(retrievalAction)
    , _partLocation(partLocation)
    , _minimum(minimum)
{
    _description = QString("retrieve-message-part:partLocation-id=%1;minimumBytes=%2")
            .arg(_partLocation.toString(true))
            .arg(_minimum);
}

RetrieveMessagePartRange::~RetrieveMessagePartRange()
{
}

void RetrieveMessagePartRange::execute()
{
     _retrievalAction->retrieveMessagePartRange(_partLocation, _minimum);
}

QMailServiceAction* RetrieveMessagePartRange::serviceAction() const
{
    return _retrievalAction;
}

/*
  RetrieveMessageRange
*/
RetrieveMessageRange::RetrieveMessageRange(QMailRetrievalAction *retrievalAction,
                                         const QMailMessageId &messageId, uint minimum)
    : EmailAction()
    , _retrievalAction(retrievalAction)
    , _messageId(messageId)
    , _minimum(minimum)
{
    _description = QString("retrieve-message-range:message-id=%1;minimumBytes=%2")
            .arg(_messageId.toULongLong())
            .arg(_minimum);
}

RetrieveMessageRange::~RetrieveMessageRange()
{
}

void RetrieveMessageRange::execute()
{
     _retrievalAction->retrieveMessageRange(_messageId, _minimum);
}

QMailServiceAction* RetrieveMessageRange::serviceAction() const
{
    return _retrievalAction;
}

/*
  RetrieveMessages
*/
RetrieveMessages::RetrieveMessages(QMailRetrievalAction *retrievalAction,
                                   const QMailMessageIdList &messageIds,
                                   QMailRetrievalAction::RetrievalSpecification spec)
    : EmailAction()
    , _retrievalAction(retrievalAction)
    , _messageIds(messageIds)
    , _spec(spec)
{
    QString idsList = idListToString(_messageIds);
    _description = QString("retrieve-messages:message-ids=%1").arg(idsList);
}

RetrieveMessages::~RetrieveMessages()
{
}

void RetrieveMessages::execute()
{
    _retrievalAction->retrieveMessages(_messageIds, _spec);
}

QMailServiceAction* RetrieveMessages::serviceAction() const
{
    return _retrievalAction;
}

/*
  Synchronize
*/
Synchronize::Synchronize(QMailRetrievalAction* retrievalAction, const QMailAccountId& id)
        : EmailAction()
        , _retrievalAction(retrievalAction)
        , _accountId(id)
{
    _description = QString("synchronize:account-id=%1").arg(_accountId.toULongLong());
}

Synchronize::~Synchronize()
{
}

void Synchronize::execute()
{
    _retrievalAction->synchronize(_accountId, 20);
}

QMailServiceAction* Synchronize::serviceAction() const
{
    return _retrievalAction;
}

/*
    TransmitMessages
*/
TransmitMessages::TransmitMessages(QMailTransmitAction* transmitAction, const QMailAccountId &id)
    : EmailAction()
    , _transmitAction(transmitAction)
    , _accountId(id)
{
    _description = QString("transmit-messages:account-id=%1").arg(_accountId.toULongLong());
}

TransmitMessages::~TransmitMessages()
{
}

void TransmitMessages::execute()
{
    _transmitAction->transmitMessages(_accountId);
}

QMailServiceAction* TransmitMessages::serviceAction() const
{
    return _transmitAction;
}


