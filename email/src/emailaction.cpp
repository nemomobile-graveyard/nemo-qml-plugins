/*
 * Copyright (C) 2013 Jolla Ltd.
 * Contact: Valerio Valerio <valerio.valerio@jollamobile.com>
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "emailaction.h"

QString idListToString(const QMailMessageIdList &ids)
{
    QString idsList;
    int idsCount = ids.count();
    int interatorPos = 0;
    foreach(QMailMessageId id, ids) {
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
EmailAction::EmailAction(QMailServiceAction *action, bool onlineAction)
    : serviceAction(action), _onlineAction(onlineAction)
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

DeleteMessages::DeleteMessages(QMailServiceAction* action, const QMailMessageIdList &ids)
    : EmailAction(action), _ids(ids)
{
    QString idsList = idListToString(_ids);
    _description = QString("delete-messages:message-ids=%1").arg(idsList);
}

DeleteMessages::~DeleteMessages()
{
}

void DeleteMessages::execute()
{
    QMailStorageAction* storageAction = qobject_cast<QMailStorageAction*>(serviceAction);
    Q_ASSERT_X(storageAction, Q_FUNC_INFO, "Cast failed");
    storageAction->deleteMessages(_ids);
}

/*
  ExportUpdates
*/
ExportUpdates::ExportUpdates(QMailServiceAction *action, const QMailAccountId &id)
    : EmailAction(action), _accountId(id)
{
    _description = QString("exporting-updates:account-id=%1").arg(_accountId.toULongLong());
}

ExportUpdates::~ExportUpdates()
{
}

void ExportUpdates::execute()
{
    QMailRetrievalAction* retrievalAction = qobject_cast<QMailRetrievalAction*>(serviceAction);
    Q_ASSERT_X(retrievalAction, Q_FUNC_INFO, "Cast failed");
    retrievalAction->exportUpdates(_accountId);
}

/*
  FlagMessages
*/
FlagMessages::FlagMessages(QMailServiceAction *action, const QMailMessageIdList &ids,
                           quint64 setMask, quint64 unsetMask)
    : EmailAction(action)
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
    QMailStorageAction* storageAction = qobject_cast<QMailStorageAction*>(serviceAction);
    Q_ASSERT_X(storageAction, Q_FUNC_INFO, "Cast failed");
    storageAction->flagMessages(_ids, _setMask, _unsetMask);
}

/*
  OnlineCreateFolder
*/
OnlineCreateFolder::OnlineCreateFolder(QMailServiceAction *action, const QString &name
                                       , const QMailAccountId &id, const QMailFolderId &parentId)
    : EmailAction(action)
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
    QMailStorageAction* storageAction = qobject_cast<QMailStorageAction*>(serviceAction);
    Q_ASSERT_X(retrievalAction, Q_FUNC_INFO, "Cast failed");
    storageAction->onlineCreateFolder(_name, _accountId, _parentId);
}

/*
  OnlineDeleteFolder
*/
OnlineDeleteFolder::OnlineDeleteFolder(QMailServiceAction *action, const QMailFolderId &folderId)
    : EmailAction(action), _folderId(folderId)
{
    _description = QString("delete-folder:folder-id=%1").arg(_folderId.toULongLong());
}

OnlineDeleteFolder::~OnlineDeleteFolder()
{
}

void OnlineDeleteFolder::execute()
{
    QMailStorageAction* storageAction = qobject_cast<QMailStorageAction*>(serviceAction);
    Q_ASSERT_X(retrievalAction, Q_FUNC_INFO, "Cast failed");
    storageAction->onlineDeleteFolder(_folderId);
}

/*
  OnlineRenameFolder
*/
OnlineRenameFolder::OnlineRenameFolder(QMailServiceAction *action, const QMailFolderId &folderId, const QString &name)
    : EmailAction(action), _folderId(folderId), _name(name)
{
    _description = QString("rename-folder:folder-id=%1;new-name=%2").arg(_folderId.toULongLong()).arg(_name);
}

OnlineRenameFolder::~OnlineRenameFolder()
{
}

void OnlineRenameFolder::execute()
{
    QMailStorageAction* storageAction = qobject_cast<QMailStorageAction*>(serviceAction);
    Q_ASSERT_X(retrievalAction, Q_FUNC_INFO, "Cast failed");
    storageAction->onlineRenameFolder(_folderId, _name);
}

/*
  RetrieveMessageList
*/
RetrieveMessageList::RetrieveMessageList(QMailServiceAction* action, const QMailAccountId& id,
                    const QMailFolderId &folderId, uint minimum)
    : EmailAction(action)
    , _accountId(id)
    , _folderId(folderId)
    , _minimum(minimum)
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
    QMailRetrievalAction* retrievalAction = qobject_cast<QMailRetrievalAction*>(serviceAction);
    Q_ASSERT_X(retrievalAction, Q_FUNC_INFO, "Cast failed");
    retrievalAction->retrieveMessageList(_accountId, _folderId, _minimum);
}

/*
  RetrieveFolderList
*/
RetrieveFolderList::RetrieveFolderList(QMailServiceAction* action, const QMailAccountId& id,
                                       const QMailFolderId &folderId, uint descending )
    : EmailAction(action)
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
    QMailRetrievalAction* retrievalAction = qobject_cast<QMailRetrievalAction*>(serviceAction);
    Q_ASSERT_X(retrievalAction, Q_FUNC_INFO, "Cast failed");
    retrievalAction->retrieveFolderList(_accountId, _folderId, _descending);
}

/*
  Synchronize
*/
Synchronize::Synchronize(QMailServiceAction* action, const QMailAccountId& id)
        : EmailAction(action), _accountId(id)
{
    _description = QString("synchronize:account-id=%1").arg(_accountId.toULongLong());
}

Synchronize::~Synchronize()
{
}

void Synchronize::execute()
{
    QMailRetrievalAction* retrievalAction = qobject_cast<QMailRetrievalAction*>(serviceAction);
    Q_ASSERT_X(retrievalAction, Q_FUNC_INFO, "Cast failed");
    retrievalAction->synchronize(_accountId, 20);
}

/*
    TransmitMessages
*/
TransmitMessages::TransmitMessages(QMailServiceAction *action, const QMailAccountId &id)
    : EmailAction(action), _accountId(id)
{
    _description = QString("transmit-messages:account-id=%1").arg(_accountId.toULongLong());
}

TransmitMessages::~TransmitMessages()
{
}

void TransmitMessages::execute(){
    QMailTransmitAction* transmitAction = qobject_cast<QMailTransmitAction*>(serviceAction);
    Q_ASSERT_X(transmitAction, Q_FUNC_INFO, "Cast failed");
    transmitAction->transmitMessages(_accountId);
}

