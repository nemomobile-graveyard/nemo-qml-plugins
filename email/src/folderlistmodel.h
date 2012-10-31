/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 	
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef FOLDERLISTMODEL_H
#define FOLDERLISTMODEL_H

#include <qmailfolder.h>
#include <qmailaccount.h>

#include <QAbstractListModel>

class FolderListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit FolderListModel (QObject *parent = 0);
    ~FolderListModel();

    enum Role {
        FolderName = Qt::UserRole + 1,
        FolderId = Qt::UserRole + 2,
        FolderUnreadCount = Qt::UserRole + 3,
        FolderServerCount = Qt::UserRole + 4,
        Index = Qt::UserRole + 5,
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;

    Q_INVOKABLE void setAccountKey (QVariant id);
    Q_INVOKABLE QStringList folderNames();
    Q_INVOKABLE QVariant folderId(int index);
    Q_INVOKABLE int indexFromFolderId(QVariant vFolderId);
    Q_INVOKABLE QVariant folderServerCount(QVariant vFolderId);
    Q_INVOKABLE QVariant inboxFolderId ();
    Q_INVOKABLE QVariant inboxFolderName();
    Q_INVOKABLE int totalNumberOfFolders();

private:
    QMailFolderIdList m_mailFolderIds;
};

#endif
