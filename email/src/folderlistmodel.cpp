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

#include <qmailnamespace.h>
#include <qmailaccount.h>
#include <qmailfolder.h>
#include <qmailmessage.h>
#include <qmailmessagekey.h>
#include <qmailstore.h>

#include "folderlistmodel.h"


FolderListModel::FolderListModel(QObject *parent) :
    QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles.insert(FolderName, "folderName");
    roles.insert(FolderId, "folderId");
    roles.insert(FolderUnreadCount, "folderUnreadCount");
    roles.insert(FolderServerCount, "folderServerCount");
    setRoleNames(roles);
}

FolderListModel::~FolderListModel()
{
}

int FolderListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_mailFolderIds.count();
}

QVariant FolderListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() > m_mailFolderIds.count())
        return QVariant();

    
    QMailFolder folder(m_mailFolderIds[index.row()]);
    if (role == FolderName) {
        return folder.displayName();
    }
    else if (role == FolderId) {
        return m_mailFolderIds[index.row()];
    } 
    else if (role == FolderUnreadCount) {
        QMailMessageKey parentFolderKey(QMailMessageKey::parentFolderId(m_mailFolderIds[index.row()]));
        QMailMessageKey unreadKey(QMailMessageKey::status(QMailMessage::Read, QMailDataComparator::Excludes));
        return (QMailStore::instance()->countMessages(parentFolderKey & unreadKey));
    }
    else if (role == FolderServerCount) {
        return (folder.serverCount());
    }

    return QVariant();
}

void FolderListModel::setAccountKey(QVariant id)
{
  // Get all the folders belonging to this email account
    QMailAccountId accountId = id.value<QMailAccountId>();
    QMailFolderKey key = QMailFolderKey::parentAccountId(accountId);
    m_mailFolderIds = QMailStore::instance()->queryFolders(key);
}

QStringList FolderListModel::folderNames()
{
    QStringList folderNames;
    foreach (QMailFolderId fId, m_mailFolderIds) {
        QMailFolder folder(fId);
        QMailMessageKey parentFolderKey(QMailMessageKey::parentFolderId(fId));
        QMailMessageKey unreadKey(QMailMessageKey::status(QMailMessage::Read, QMailDataComparator::Excludes));
        int numberOfMessages = QMailStore::instance()->countMessages(parentFolderKey & unreadKey);
        QString displayName = folder.displayName();
        if (numberOfMessages > 0) {
            displayName = displayName + " (" + QString::number(numberOfMessages) + ")";
        }
        folderNames << displayName;
    }
    return folderNames;
}

QVariant FolderListModel::folderId(int index)
{
    if (index < 0 || index >= m_mailFolderIds.count())
        return QVariant();

    return m_mailFolderIds[index];
}

int FolderListModel::indexFromFolderId(QVariant vFolderId)
{
    QMailFolderId folderId;

    if (vFolderId == 0)
        folderId = QMailFolderId(QMailFolder::InboxFolder);
    else
        folderId = vFolderId.value<QMailFolderId>();

    for (int i = 0; i < m_mailFolderIds.size(); i ++) {
        if (folderId == m_mailFolderIds[i])
            return i;
    }
    return -1;
}

QVariant FolderListModel::folderServerCount(QVariant vFolderId)
{
    QMailFolderId folderId = vFolderId.value<QMailFolderId>();
    if (!folderId.isValid())
        return 0;

    QMailFolder folder (folderId);
    return (folder.serverCount());
}

//TODO: Remove
//We can't assume that all create folder models will have an Inbox
//E.g, list of childs under some particular folder
//This can be get from the emailAgent
QVariant FolderListModel::inboxFolderId()
{
    for (int i = 0; i < m_mailFolderIds.size(); i++) {
        QMailFolder folder(m_mailFolderIds[i]);
        if (QString::compare(folder.displayName(), "INBOX", Qt::CaseInsensitive) == 0)
            return m_mailFolderIds[i];
    }
    return QVariant();
}

//TODO: investigate if this is really needed
//seems redundant
QVariant FolderListModel::inboxFolderName()
{
    for (int i = 0; i < m_mailFolderIds.size(); i++) {
        QMailFolder folder(m_mailFolderIds[i]);
        QString folderName = folder.displayName();
        if (QString::compare(folderName, "INBOX", Qt::CaseInsensitive) == 0)
            return folderName;
    }
    return QVariant("");
}

int FolderListModel::totalNumberOfFolders()
{
    return m_mailFolderIds.count();
}

QVariant FolderListModel::folderUnreadCount(QVariant folderId)
{
    int folderIndex = indexFromFolderId(folderId);

    if (folderIndex < 0)
        return 0;

    return data(index(folderIndex), FolderUnreadCount);
}
