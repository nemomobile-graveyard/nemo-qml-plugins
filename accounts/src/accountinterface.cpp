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

#include "accountinterface.h"
#include "accountinterface_p.h"

#include "providerinterface.h"

//libaccounts-qt
#include <Accounts/Manager>
#include <Accounts/Account>
#include <Accounts/Service>

AccountInterfacePrivate::AccountInterfacePrivate(AccountInterface *parent, Accounts::Account *acc)
    : QObject(parent), q(parent), account(acc), oldIdentifier(acc->id()),
      oldEnabled(acc->enabled()), oldDisplayName(acc->displayName()),
      status(AccountInterface::Synced), error(AccountInterface::NoError)
{
    // enumerate the configuration values
    QVariantMap allValues;
    QStringList allKeys = account->allKeys();
    foreach (const QString &key, allKeys) {
        QVariant currValue;
        account->value(key, currValue); // pass by ref.
        allValues.insert(key, currValue);
    }
    oldConfigurationValues = allValues;

    // enumerate the supported services and the associated credentials
    QVariantMap allCredentials;
    Accounts::ServiceList supportedServices = account->services();
    for (int i = 0; i < supportedServices.size(); ++i) {
        Accounts::Service currService = supportedServices.at(i);
        QString serviceName = currService.name();
        supportedServiceNames.append(serviceName);
        account->selectService(currService);
        int currCredId = account->credentialsId();
        allCredentials.insert(serviceName, currCredId);
        account->selectService(Accounts::Service());
    }
    int currCredId = account->credentialsId();
    allCredentials.insert(QString(), currCredId); // and the global credential id.
    oldIdentityIdentifiers = allCredentials;

    // connect up our signals.
    connect(account, SIGNAL(enabledChanged(QString,bool)), this, SLOT(enabledHandler(QString,bool)));
    connect(account, SIGNAL(displayNameChanged(QString)), this, SLOT(displayNameChangedHandler()));
    connect(account, SIGNAL(synced()), this, SLOT(handleSynced()));
    connect(account, SIGNAL(removed()), this, SLOT(invalidate()));
    connect(account, SIGNAL(destroyed()), this, SLOT(invalidate()));
}

AccountInterfacePrivate::~AccountInterfacePrivate()
{
}

void AccountInterfacePrivate::enabledHandler(const QString &serviceName, bool newEnabled)
{
    if (serviceName.isEmpty()) {
        if (newEnabled != oldEnabled) {
            oldEnabled = newEnabled;
            emit q->enabledChanged();
        }
    } else {
        // note: we can't cache the values on set, and check here,
        // because we can never know the complete set of enabled/disabled
        // service names at the same time :-/
        // So, the semantics of enabledServiceNames is a bit different: requires sync() to emit.
        if (newEnabled)
            enabledServiceNames.append(serviceName);
        else
            enabledServiceNames.removeAll(serviceName);
        emit q->enabledServiceNamesChanged();
    }
}

void AccountInterfacePrivate::displayNameChangedHandler()
{
    if (oldDisplayName != account->displayName()) {
        oldDisplayName = account->displayName();
        emit q->displayNameChanged();
    }
}

void AccountInterfacePrivate::invalidate()
{
    // NOTE: the Accounts::Manager instance ALWAYS owns the account pointer.
    // If the manager gets deleted while the AccountInterface instance is
    // alive, we need to ensure that invalidate() gets called also.
    // We also invalidate the interface if the account itself gets removed
    // from the accounts database.
    account = 0;
    setStatus(AccountInterface::Invalid);
}

void AccountInterfacePrivate::handleSynced()
{
    if (status == AccountInterface::SyncInProgress) {
        // check to see if the id was updated
        int tmp = q->identifier();
        if (oldIdentifier != tmp) {
            oldIdentifier = tmp;
            emit q->identifierChanged();
        }

        // check to see if the identity identifiers were updated (possibly due to error)
        QVariantMap allCredentials;
        Accounts::ServiceList supportedServices = account->services();
        for (int i = 0; i < supportedServices.size(); ++i) {
            Accounts::Service currService = supportedServices.at(i);
            QString serviceName = currService.name();
            account->selectService(currService);
            int currCredId = account->credentialsId();
            allCredentials.insert(serviceName, currCredId);
            account->selectService(Accounts::Service());
        }
        allCredentials.insert(QString(), account->credentialsId()); // and the global credential id.
        if (oldIdentityIdentifiers != allCredentials) {
            oldIdentityIdentifiers = allCredentials;
            emit q->identityIdentifiersChanged();
        }

        // check to see if the configuration values were updated
        QVariantMap allValues;
        QStringList allKeys = account->allKeys();
        foreach (const QString &key, allKeys) {
            QVariant currValue;
            account->value(key, currValue); // pass by ref.
            allValues.insert(key, currValue);
        }
        if (oldConfigurationValues != allValues) {
            oldConfigurationValues = allValues;
            emit q->configurationValuesChanged();
        }

        // and update our status.
        setStatus(AccountInterface::Synced);
    }
}

void AccountInterfacePrivate::setStatus(AccountInterface::Status newStatus)
{
    if (status == AccountInterface::Invalid)
        return; // once invalid, cannot be restored.

    if (status != newStatus) {
        status = newStatus;
        emit q->statusChanged();
    }
}

//-----------------------------------

/*!
    \qmltype Account
    \instantiates AccountInterface
    \inqmlmodule org.nemomobile.accounts 1
    \brief Used to create or modify an account for a service provider

    The Account type is a non-visual type which is intended for use
    by privileged applications or account-provider plugins.  It allows
    the details of an account to be specified, and saved to the system
    accounts database.

    An Account can be created by calling AccountManager.createAccount()
    An Account can be removed by calling AccountManager.removeAccount()
    An Account can be updated by calling Account.sync()

    An Account can be associated with an Identity via the Identity's
    id.  Any modifications to any property of an account will have no
    effect until the modifications are saved to the database by calling
    sync().

    Example usage:

    \qml
        import org.nemomobile.accounts 1.0

        Item {
            id: root
            property AccountManager acm: AccountManager { }
            property QtObject account

            Component.onCompleted: {
                account = acm.createAccount("google")
                account.statusChanged.connect(handleStatusChange)
                account.enabled = true
                account.displayName = "example account"
                account.enableAccountWithService("google-talk")
                account.setConfigurationValue("AwayMessage", "I'm away!")
                account.sync() // triggers db write.
            }

            function handleStatusChange() {
                if (account.status == Account.Error) {
                    // handle error
                } else if (account.status == Account.Synced) {
                    // successfully written to db.
                }
            }
        }
    \endqml
*/

AccountInterface::AccountInterface(Accounts::Account *account, QObject *parent)
    : QObject(parent), d(new AccountInterfacePrivate(this, account))
{
}

AccountInterface::~AccountInterface()
{
}

Accounts::Account *AccountInterface::account()
{
    return d->account; // helper for AccountManagerInterface only.
}

/*!
    \qmlmethod void Account::sync()

    Writes any outstanding local modifications to the database.
    The operation may be either synchronous or asynchronous
    depending on whether the database is currently locked or
    open for writing.  The account will transition to the
    \c{SyncInProgress} status and remain with that status for
    the duration of the synchronisation operation.

    Calling this function will have no effect if the account is
    invalid or if a previous synchronisation operation is in
    progress.
*/
void AccountInterface::sync()
{
    if (!d->account || d->status == AccountInterface::SyncInProgress)
        return;

    d->account->setCredentialsId(d->oldIdentifier);
    d->setStatus(AccountInterface::SyncInProgress);
    d->account->sync();
}

/*!
    \qmlmethod void Account::setConfigurationValue(const QString &key, const QVariant &value)
    Sets the account's configuration settings value for the key \a key
    to the value \a value.
*/
void AccountInterface::setConfigurationValue(const QString &key, const QVariant &value)
{
    if (!d->account || d->status == AccountInterface::Invalid || d->status == AccountInterface::SyncInProgress)
        return;

    d->oldConfigurationValues.insert(key, value);
    d->account->setValue(key, value);
    d->setStatus(AccountInterface::Modified);
    emit configurationValuesChanged();
}

/*!
    \qmlmethod void Account::removeConfigurationValue(const QString &key)
    Removes the key \a key and any associated values from the
    configuration settings of the account.
*/
void AccountInterface::removeConfigurationValue(const QString &key)
{
    if (!d->account || d->status == AccountInterface::Invalid || d->status == AccountInterface::SyncInProgress)
        return;

    d->oldConfigurationValues.remove(key);
    d->account->remove(key);
    d->setStatus(AccountInterface::Modified);
    emit configurationValuesChanged();
}

bool AccountInterface::supportsServiceType(const QString &serviceType)
{
    if (!d->account)
        return false;
    return d->account->supportsService(serviceType);
}

/*!
    \qmlmethod void Account::enableAccountWithService(const QString &serviceName)

    Enables the account with the service identified by the given \a serviceName.

    If the service does not exist, or this account does not support the service,
    or the status of the account is either Invalid or SyncInProgress, the operation
    will silently fail.

    Note: this method will have no effect until sync() is called!
*/
void AccountInterface::enableAccountWithService(const QString &serviceName)
{
    if (!d->account || d->status == AccountInterface::Invalid || d->status == AccountInterface::SyncInProgress)
        return;

    Accounts::Service srv;
    Accounts::ServiceList supportedServices = d->account->services();
    for (int i = 0; i < supportedServices.size(); ++i) {
        Accounts::Service currService = supportedServices.at(i);
        if (serviceName == currService.name()) {
            srv = currService;
            break;
        }
    }

    if (!srv.isValid())
        return;

    // yeah, I know.
    d->account->selectService(srv);
    d->account->setEnabled(true);
    d->account->selectService(Accounts::Service());
    d->setStatus(AccountInterface::Modified);
}


/*!
    \qmlmethod void Account::disableAccountWithService(const QString &serviceName)

    Disables the account with the service identified by the given \a serviceName.

    If the service does not exist, or this account does not support the service,
    or the status of the account is either Invalid or SyncInProgress, the operation
    will silently fail.

    Note: this method will have no effect until sync() is called!
*/
void AccountInterface::disableAccountWithService(const QString &serviceName)
{
    if (!d->account || d->status == AccountInterface::Invalid || d->status == AccountInterface::SyncInProgress)
        return;

    Accounts::Service srv;
    Accounts::ServiceList supportedServices = d->account->services();
    for (int i = 0; i < supportedServices.size(); ++i) {
        Accounts::Service currService = supportedServices.at(i);
        if (serviceName == currService.name()) {
            srv = currService;
            break;
        }
    }

    if (!srv.isValid())
        return;

    // yeah, I know.
    d->account->selectService(srv);
    d->account->setEnabled(false);
    d->account->selectService(Accounts::Service());
    d->setStatus(AccountInterface::Modified);
}



/*!
    \qmlproperty bool Account::enabled
    This property will be true if the account can be used, or false if it cannot.

    The account should be enabled if the details specified for it are valid.
    An account may need valid credentials associated with it before it can be
    enabled.
*/

bool AccountInterface::enabled() const
{
    if (!d->account)
        return false;
    return d->oldEnabled;
}

void AccountInterface::setEnabled(bool e)
{
    if (!d->account || d->status == AccountInterface::Invalid || d->status == AccountInterface::SyncInProgress)
        return;
    d->oldEnabled = e;
    d->account->setEnabled(e);
    d->setStatus(AccountInterface::Modified);
    emit enabledChanged();
}

/*!
    \qmlproperty int Account::identifier
    This property contains the identifier of the Account.

    The value of the property will be zero if the Account is a new, unsynced
    account.  If the Account has been saved in the system accounts database,
    it will be non-zero.
*/

int AccountInterface::identifier() const
{
    if (!d->account)
        return 0;
    return d->account->id();
}

/*!
    \qmlproperty QVariantMap Account::identityIdentifiers

    The property contains the identifiers of each Identity (signon credentials)
    associated with the account, mapped to the services for which they are valid.
    The account-global credentials are mapped to an empty service name.
*/

QVariantMap AccountInterface::identityIdentifiers() const
{
    if (!d->account)
        return QVariantMap();

    // Note: we don't return d->account->credentialsId();
    // as that doesn't get updated until sync().
    return d->oldIdentityIdentifiers;
}

/*!
    \qmlmethod int Account::identityIdentifier(const QString &serviceName)

    Returns the identifier of the identity (sign-on credentials) which should
    be used by this account when signing on to the service identified by the
    specified \a serviceName.

    If \a serviceName is empty, the identifier of the account-global credentials
    will be returned.
*/
int AccountInterface::identityIdentifier(const QString &serviceName) const
{
    if (!d->account)
        return 0;
    return d->oldIdentityIdentifiers.value(serviceName).value<int>();
}

/*!
    \qmlmethod void Account::setIdentityIdentifier(int id, const QString &serviceName)

    Sets the identifier of the identity (sign-on credentials) which should
    be used by this account when signing on to the service identified by the
    specified \a serviceName to the given \a id.

    If \a serviceName is empty, the identifier of the account-global credentials
    will be updated to \a id.  If \a serviceName doesn't identity a valid service,
    or the service is not supported by this account, this operation will silently
    fail.
*/
void AccountInterface::setIdentityIdentifier(int id, const QString &serviceName)
{
    if (!d->account || d->status == AccountInterface::Invalid || d->status == AccountInterface::SyncInProgress)
        return;

    Accounts::Service srv;
    if (!serviceName.isEmpty()) { // service-specific credentials
        Accounts::ServiceList supportedServices = d->account->services();
        for (int i = 0; i < supportedServices.size(); ++i) {
            Accounts::Service currService = supportedServices.at(i);
            if (serviceName == currService.name()) {
                srv = currService;
                break;
            }
        }

        if (!srv.isValid())
            return; // silently fail.
    }

    d->account->selectService(srv);
    d->account->setCredentialsId(id);
    d->account->selectService(Accounts::Service());

    d->oldIdentityIdentifiers.insert(serviceName, id);
    d->setStatus(AccountInterface::Modified);
    emit identityIdentifiersChanged();
}

/*!
    \qmlproperty string Account::providerName
    This property contains the name of the service provider with which
    the account is valid.

    An account provider plugin will provide a \c{.provider} file in
    \c{/usr/share/accounts/providers} which specifies the name of the
    provider.

    A valid provider name must be specified when creating an account
    via AccountManager.createAccount().
*/

QString AccountInterface::providerName() const
{
    if (!d->account)
        return QString();
    return d->account->providerName();
}

/*!
    \qmlproperty string Account::displayName
    This property contains the display name of the account

    The display name is the name of the account which should be
    displayed to users in selection lists, edit dialogues, and
    other user-interface contexts.
*/

QString AccountInterface::displayName() const
{
    if (!d->account)
        return QString();
    return d->oldDisplayName;
}

void AccountInterface::setDisplayName(const QString &dn)
{
    if (!d->account || d->status == AccountInterface::Invalid || d->status == AccountInterface::SyncInProgress)
        return;

    d->oldDisplayName = dn;
    d->account->setDisplayName(dn);
    d->setStatus(AccountInterface::Modified);
    emit displayNameChanged();
}

/*!
    \qmlproperty QStringList Account::supportedServiceNames
    This property contains the names of services supported by the account

    Every service provided by a provider has a service name which is
    specified in the \c{.service} file located at
    \c{/usr/share/accounts/services} which is installed by the account
    provider plugin.
*/

QStringList AccountInterface::supportedServiceNames() const
{
    if (!d->account)
        return QStringList();
    return d->supportedServiceNames;
}


/*!
    \qmlproperty QStringList Account::enabledServiceNames
    This property contains the names of services for which the account is
    enabled.

    An account may be enabled with any service it supports, by calling
    enableAccountWithService().  It may be disabled with a service by
    calling disableAccountWithService().

    During account creation, the account should be enabled with any
    service for which it is valid, or as specified by the user.
*/

QStringList AccountInterface::enabledServiceNames() const
{
    if (!d->account)
        return QStringList();
    return d->enabledServiceNames;
}



/*!
    \qmlproperty QVariantMap Account::configurationValues
    This property contains the configuration settings of the account

    Some default settings are usually specified in the \c{.service}
    file installed by the account provider plugin.  Other settings
    may be specified directly for an account.
*/

QVariantMap AccountInterface::configurationValues() const
{
    if (!d->account)
        return QVariantMap();
    return d->oldConfigurationValues;
}

void AccountInterface::setConfigurationValues(const QVariantMap &values)
{
    if (!d->account || d->status == AccountInterface::Invalid || d->status == AccountInterface::SyncInProgress)
        return;

    QStringList allKeys = values.keys();
    foreach (const QString &key, allKeys) {
        const QVariant &currValue = values.value(key);
        d->account->remove(key);
        d->account->setValue(key, currValue);
    }

    d->oldConfigurationValues = values;
    d->setStatus(AccountInterface::Modified);
    emit configurationValuesChanged();
}


/*!
    \qmlproperty Account::Status Account::status
    This property contains the current database-sync status of the account

    An account may have any of five statuses:
    \table
        \header
            \li Status
            \li Description
        \row
            \li Synced
            \li No outstanding local modifications to the account have occurred since last sync().  Any previous sync() calls have completed.
        \row
            \li SyncInProgress
            \li Any outstanding local modifications are currently being written to the database due to a call to sync().  No local property modifications may occur while the account has this status.
        \row
            \li Modified
            \li Local modifications to the account have occurred since last sync().  In order to persist the changes to the database, sync() must be called.  Note that if another process modifies the canonical (database) version of the account, no signal will be emitted and thus the status of the local account representation will NOT automatically change to Modified.
        \row
            \li Error
            \li An error occurred during account creation or synchronisation.
        \row
            \li Invalid
            \li The account has been removed from the database and is no longer valid.
    \endtable

    Connecting to the account's statusChanged() signal is the usual
    way to handle database synchronisation events.
*/

AccountInterface::Status AccountInterface::status() const
{
    return d->status;
}


/*!
    \qmlproperty Account::Error Account::error
    This property contains the most recent error which occurred during
    account creation or synchronisation.

    Note that the error will NOT automatically return to \c{NoError}
    if subsequent synchronisation operations succeed.
*/

AccountInterface::ErrorType AccountInterface::error() const
{
    return d->error;
}


/*!
    \qmlproperty string Account::errorMessage
    This property contains the error message associated with the most
    recent error which occurred during account creation or synchronisation.

    Note that the error message will NOT automatically return to
    being empty if subsequent synchronisation operations succeed.
*/

QString AccountInterface::errorMessage() const
{
    return d->errorMessage;
}
