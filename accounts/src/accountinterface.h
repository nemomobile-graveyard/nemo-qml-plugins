/*
 * Copyright (C) 2012 Jolla Mobile <chris.adams@jollamobile.com>
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

#ifndef ACCOUNTINTERFACE_H
#define ACCOUNTINTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include <QtCore/QString>

//libaccounts-qt
#include <Accounts/Account>
#include <Accounts/Error>

class AccountInterfacePrivate;
class AccountManagerInterface;

/*
 * AccountInterface
 *
 * Exposes the functionality of a particular account to QML.
 * It is intended to be used by the settings application
 * and by 3rd party extension plugins for the settings application.
 *
 * Note that modifying the AccountInterface returned by the
 * AccountManagerInterface will have no effect until you call the
 * sync() method of the AccountInterface.
 *
 * eg:
 *
 * import org.nemomobile.accounts 1.0
 *
 * Item {
 *    id: root
 *    property AccountManager acm: AccountManager { }
 *    Component.onCompleted: {
 *        var account = acm.createAccount("google")
 *        account.enabled = true
 *        account.displayName = "example account"
 *        account.enableAccountWithService("google-talk")
 *        account.setConfigurationValue("AwayMessage", "I'm away!")
 *        account.sync() // triggers db write.
 *    }
 * }
 *
 * Once you have created an account, you should create
 * an identity/credentials in libsignon, and the set the identity into the
 * newly created account via its identityIdentifier property.
 */
class AccountInterface : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(int identifier READ identifier NOTIFY identifierChanged)
    Q_PROPERTY(QVariantMap identityIdentifiers READ identityIdentifiers NOTIFY identityIdentifiersChanged)
    Q_PROPERTY(QString providerName READ providerName CONSTANT)

    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged)

    Q_PROPERTY(QStringList supportedServiceNames READ supportedServiceNames CONSTANT)
    Q_PROPERTY(QStringList enabledServiceNames READ enabledServiceNames NOTIFY enabledServiceNamesChanged)

    Q_PROPERTY(QVariantMap configurationValues READ configurationValues WRITE setConfigurationValues NOTIFY configurationValuesChanged)

    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(ErrorType error READ error NOTIFY errorChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

    Q_ENUMS(Status)
    Q_ENUMS(ErrorType)

public:
    enum Status {
        Synced = 0,
        SyncInProgress,
        Modified,
        Error,
        Invalid
    };

    enum ErrorType {
        NoError                 = Accounts::Error::NoError,
        UnknownError            = Accounts::Error::Unknown,
        DatabaseError           = Accounts::Error::Database,
        DeletedError            = Accounts::Error::Deleted,
        DatabaseLockedError     = Accounts::Error::DatabaseLocked,
        AccountNotFoundError    = Accounts::Error::AccountNotFound
    };

public:
    AccountInterface(Accounts::Account *account, QObject *parent = 0);
    ~AccountInterface();

    // database sync
    Q_INVOKABLE void sync();

    // invokable api.
    Q_INVOKABLE void setConfigurationValue(const QString &key, const QVariant &value);
    Q_INVOKABLE void removeConfigurationValue(const QString &key);

    Q_INVOKABLE bool supportsServiceType(const QString &serviceType);
    Q_INVOKABLE void enableAccountWithService(const QString &serviceName);
    Q_INVOKABLE void disableAccountWithService(const QString &serviceName);

    Q_INVOKABLE int identityIdentifier(const QString &serviceName = QString()) const;
    Q_INVOKABLE void setIdentityIdentifier(int id, const QString &serviceName = QString());

    // property accessors.
    bool enabled() const;
    void setEnabled(bool e);
    int identifier() const;
    QVariantMap identityIdentifiers() const;
    QString displayName() const;
    void setDisplayName(const QString &dn);
    QString providerName() const;
    QStringList supportedServiceNames() const;
    QStringList enabledServiceNames() const;
    QVariantMap configurationValues() const;
    void setConfigurationValues(const QVariantMap &values);

    Status status() const;
    ErrorType error() const;
    QString errorMessage() const;

Q_SIGNALS:
    void enabledChanged();
    void identifierChanged();
    void identityIdentifiersChanged();
    void displayNameChanged();
    void enabledServiceNamesChanged();
    void configurationValuesChanged();
    void statusChanged();
    void errorChanged();
    void errorMessageChanged();

private:
    Accounts::Account *account();
    AccountInterfacePrivate *d;
    friend class AccountInterfacePrivate;
    friend class AccountManagerInterface;
};

#endif
