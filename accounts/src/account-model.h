/*
 * Copyright (C) 2012 Jolla Ltd.
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Nemo Mobile nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 */

#ifndef ACCOUNTMODEL_H
#define ACCOUNTMODEL_H

//accounts-qt
#include <Accounts/Manager>

//Qt
#include <QAbstractTableModel>
#include <QMap>
#include <QVariant>

class ServiceAccountInterface;
class ProviderInterface;

/*!
 * The Account Model is the model for created accounts
 */

class AccountModel : public QAbstractListModel
{
    Q_OBJECT
    class AccountModelPrivate;

public:

    enum Columns {
        AccountIdColumn,
        AccountDisplayNameColumn,
        AccountIconColumn,
        ProviderNameColumn,
        ProviderDisplayNameColumn,
        ColumnCount
    };

    enum Roles{
        AccountIdRole = Qt::UserRole + 1,
        AccountDisplayNameRole,
        AccountIconRole,
        ProviderNameRole,
        ProviderDisplayNameRole,
        ColumnCountRole
    };


    AccountModel(QObject *parent = 0);
    virtual ~AccountModel();

    /*!
     * Derived from QAbstractListModel - delivers the number of rows in the
     * model.
     */
    int rowCount(const QModelIndex &index = QModelIndex()) const;

    /*!
     * Derived from QAbstractListModel - returns the number of columns in the
     * model.
     */
    int columnCount(const QModelIndex &index) const;

    /*!
     * Fetches data depending on the index and the role provided.
     */
    QVariant data(const QModelIndex &index, int role) const;

    /*!
    *  Maps all the roles with a QString so that qmlList can access the model
    */
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role) const;

    Q_INVOKABLE ServiceAccountInterface *serviceAccount(int accountId, const QString &serviceName, QObject *parent = 0);
    Q_INVOKABLE ProviderInterface *provider(const QString &providerName, QObject *parent = 0);
    Q_INVOKABLE ProviderInterface *provider(int accountId, QObject *parent = 0);

private slots:
    void accountCreated(Accounts::AccountId id);
    void accountRemoved();
    void accountUpdated();

private:
    int getAccountIndex(Accounts::Account *account) const;

private:
    AccountModelPrivate* d_ptr;
    Q_DISABLE_COPY(AccountModel)
    Q_DECLARE_PRIVATE(AccountModel);
};
Q_DECLARE_METATYPE(Accounts::Account *)

#endif // ACCOUNTMODEL_H
