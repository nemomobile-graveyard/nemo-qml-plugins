/*
 * Copyright 2011 Intel Corporation.
 * Copyright (C) 2012 Jolla Ltd.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 	
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef EMAILACCOUNTLISTMODEL_H
#define EMAILACCOUNTLISTMODEL_H

#include <QAbstractListModel>

#include <qmailaccountlistmodel.h>
#include <qmailaccount.h>

class EmailAccountListModel : public QMailAccountListModel
{
    Q_OBJECT

public:
    explicit EmailAccountListModel (QObject *parent = 0);
    ~EmailAccountListModel();

    enum Role {
        DisplayName = Qt::UserRole + 4,
        EmailAddress = Qt::UserRole + 5,
        MailServer = Qt::UserRole + 6,
        UnreadCount = Qt::UserRole + 7,
        MailAccountId  = Qt::UserRole + 8,
        LastSynchronized = Qt::UserRole + 9,
        Index = Qt::UserRole + 10
    };

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

public slots:
    Q_INVOKABLE QVariant indexFromAccountId(QVariant id);
    Q_INVOKABLE QVariant getDisplayNameByIndex(int idx);
    Q_INVOKABLE QVariant getEmailAddressByIndex(int idx);
    Q_INVOKABLE int getRowCount();
    Q_INVOKABLE QVariant getAllEmailAddresses();
    Q_INVOKABLE QVariant getAllDisplayNames();
    Q_INVOKABLE QVariant getAccountIdByIndex(int idx);

signals:
    void accountAdded(QVariant accountId);
    void accountRemoved(QVariant accountId);
    void modelReset();

private slots:
    void onAccountsAdded(const QMailAccountIdList &);
    void onAccountsRemoved(const QMailAccountIdList &);
    void onAccountsUpdated(const QMailAccountIdList &);

private:
};

#endif
