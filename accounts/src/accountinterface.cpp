/*
 * Copyright (C) 2012 Jolla Ltd. <chris.adams@jollamobile.com>
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

#include <QtDebug>

//libaccounts-qt
#include <Accounts/Manager>
#include <Accounts/Account>
#include <Accounts/Service>

AccountInterfacePrivate::AccountInterfacePrivate(AccountInterface *parent, Accounts::Account *acc)
    : QObject(parent)
    , q(parent)
    , manager(new Accounts::Manager)
    , account(0)
    , pendingSync(false)
    , pendingInitModifications(false)
    , identifier(0)
    , enabled(false)
    , identifierPendingInit(false)
    , providerNamePendingInit(false)
    , identityIdentifiersPendingInit(false)
    , enabledPendingInit(false)
    , displayNamePendingInit(false)
    , configurationValuesPendingInit(false)
    , enabledServiceNamesPendingInit(false)
    , status(AccountInterface::Initializing)
    , error(AccountInterface::NoError)
{
    if (acc)
        setAccount(acc);
}

AccountInterfacePrivate::~AccountInterfacePrivate()
{
}

static QVariant configurationValueVariant(Accounts::Account *acc, const QString &key)
{
    QString strVal = acc->valueAsString(key);
    if (!strVal.isNull())
        return QVariant(strVal);

    quint64 uintMax = 0xffffffffffffffffU;
    quint64 uintVal = acc->valueAsUInt64(key, uintMax);
    if (uintVal != uintMax)
        return QVariant(uintVal);

    int intMax = 0xffffffff;
    int intVal = acc->valueAsInt(key, intMax);
    if (intVal != intMax)
        return QVariant(intVal);

    bool boolVal = acc->valueAsBool(key);
    return QVariant(boolVal);
}

void AccountInterfacePrivate::setAccount(Accounts::Account *acc)
{
    if (!acc) {
        qWarning() << "AccountInterface: setAccount() called with null account! Aborting operation.";
        return;
    }

    if (account) {
        qWarning() << "AccountInterface: setAccount() called but account already set! Aborting operation.";
        return;
    }

    account = acc;

    // connect up our signals.
    connect(account, SIGNAL(enabledChanged(QString,bool)), this, SLOT(enabledHandler(QString,bool)));
    connect(account, SIGNAL(displayNameChanged(QString)), this, SLOT(displayNameChangedHandler()));
    connect(account, SIGNAL(synced()), this, SLOT(handleSynced()));
    connect(account, SIGNAL(removed()), this, SLOT(invalidate()));
    connect(account, SIGNAL(destroyed()), this, SLOT(invalidate()));

    // first time read from db.  we should be in Initializing state to begin with.
    // QueuedConnection to ensure that clients have a chance to connect to state changed signals.
    QMetaObject::invokeMethod(this, "asyncQueryInfo", Qt::QueuedConnection);
}

void AccountInterfacePrivate::asyncQueryInfo()
{
    if (!account) {
        qWarning() << "AccountInterface: no account set!  Maybe you forgot to call componentComplete()?";
        setStatus(AccountInterface::Invalid);
        return;
    }

    // note that the account doesn't have a queryInfo() like Identity
    // so we just read the values directly.

    int newIdentifier = account->id();
    if (identifier != newIdentifier) {
        identifier = account->id();
        emit q->identifierChanged();
    }

    if (providerName != account->providerName()) {
        providerName = account->providerName();
        emit q->providerNameChanged();
    }

    // supported service names
    Accounts::ServiceList supportedServices = account->services();
    for (int i = 0; i < supportedServices.size(); ++i) {
        Accounts::Service currService = supportedServices.at(i);
        QString serviceName = currService.name();
        supportedServiceNames.append(serviceName);
    }
    emit q->supportedServiceNamesChanged();

    // identity identifiers
    if (identityIdentifiersPendingInit) {
        pendingInitModifications = true;
        identityIdentifiersPendingInit = false; // explicitly update this on sync if it changes
    } else {
        // enumerate the supported services and the associated credentials
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
        int currCredId = account->credentialsId();
        allCredentials.insert(QString(), currCredId); // and the global credential id.
        if (identityIdentifiers != allCredentials) {
            identityIdentifiers = allCredentials;
            emit q->identityIdentifiersChanged();
        }
    }

    // enabled
    if (enabledPendingInit) {
        pendingInitModifications = true;
    } else if (enabled != account->enabled()) {
        enabled = account->enabled();
        emit q->enabledChanged();
    }

    // display name
    if (displayNamePendingInit) {
        pendingInitModifications = true;
    } else if (displayName != account->displayName()) {
        displayName = account->displayName();
        emit q->displayNameChanged();
    }

    // configuration values
    if (configurationValuesPendingInit) {
        pendingInitModifications = true;
    } else {
        // enumerate the configuration values
        QVariantMap allValues;
        QStringList allKeys = account->allKeys();
        foreach (const QString &key, allKeys)
            allValues.insert(key, configurationValueVariant(account, key));
        if (configurationValues != allValues) {
            configurationValues = allValues;
            emit q->configurationValuesChanged();
        }
    }

    // enabled services
    if (enabledServiceNamesPendingInit) {
        QStringList tmpList = enabledServiceNames;
        enabledServiceNames.clear(); // clear them all.  this is because of the sync() semantics of service names.
        foreach (const QString &sn, tmpList) {
            if (supportedServiceNames.contains(sn))
                q->enableAccountWithService(sn);
        }
    } else {
        // nothing to do.
    }

    // do sync if required.
    if (status == AccountInterface::Invalid || status == AccountInterface::Error) {
        // error occurred during initialization, or was removed.
        // do nothing - the client will have already been notified.
    } else {
        // initialization completed successfully.
        setStatus(AccountInterface::Initialized);
        if (pendingInitModifications)
            setStatus(AccountInterface::Modified); // modifications occurred prior to initialization completion.
        if (pendingSync) {
            pendingSync = false;
            q->sync(); // the user requested sync() while we were initializing.
        }
    }
}

void AccountInterfacePrivate::enabledHandler(const QString &serviceName, bool newEnabled)
{
    // check to see if it's the "global" service name (generated by libaccounts-qt)
    // or an actual service name :-/
    if (serviceName.isEmpty() || serviceName == QString(QLatin1String("global"))) {
        if (!enabledPendingInit && newEnabled != enabled) {
            enabled = newEnabled;
            emit q->enabledChanged();
        }
    } else {
        // note: we can't cache the values on set, and check here,
        // because we can never know the complete set of enabled/disabled
        // service names at the same time :-/
        // So, the semantics of enabledServiceNames is a bit different: requires sync() to emit.
        if (newEnabled) {
            if (!enabledServiceNames.contains(serviceName)) {
                enabledServiceNames.append(serviceName);
            }
        } else {
            enabledServiceNames.removeAll(serviceName);
        }

        // note: we _always_ emit even if the content of enabledServiceNames
        // doesn't actually change as a result of this signal.
        // We simply cannot determine whether we need to or not, otherwise.
        emit q->enabledServiceNamesChanged();
    }
}

void AccountInterfacePrivate::displayNameChangedHandler()
{
    if (displayNamePendingInit)
        return; // ignore, we have local changes which will overwrite this.

    if (displayName != account->displayName()) {
        displayName = account->displayName();
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
        int newIdentifier = account->id();
        if (identifier != newIdentifier) {
            identifier = account->id();
            emit q->identifierChanged();
        }

        // check to see if the providerName was updated
        if (providerName != account->providerName()) {
            providerName = account->providerName();
            emit q->providerNameChanged();
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
        if (identityIdentifiers != allCredentials) {
            identityIdentifiers = allCredentials;
            emit q->identityIdentifiersChanged();
        }

        // check to see if the configuration values were updated
        QVariantMap allValues;
        QStringList allKeys = account->allKeys();
        foreach (const QString &key, allKeys)
            allValues.insert(key, configurationValueVariant(account, key));
        if (configurationValues != allValues) {
            configurationValues = allValues;
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

    Any modifications to any property of an account will have no effect
    until the modifications are saved to the database by calling sync().

    If the \c providerName property of the account is specified, a new
    account with the provider identified by the \c providerName will
    be created.  An example of account creation is:

    \qml
        import org.nemomobile.accounts 1.0

        Item {
            id: root
            
            Account {
                id: account
                providerName: "google"
                displayName: "example account"

                onStatusChanged: {
                    if (status == Account.Initialized) {
                        enableAccountWithService("google-talk")
                        setConfigurationValue("AwayMessage", "I'm away!")
                        sync() // trigger database write
                    } else if (status == Account.Synced) {
                        // successfully written to database
                    } else if (status == Account.Error) {
                        // handle error
                    }
                }
            }
        }
    \endqml

    If the \c identifier property of the account is specified, an existing
    account with the specified id will be loaded from the database.
    This is useful for modifying or removing an existing account.
    If the \c identifier property is specified, the \c providerName property
    may not be specified.  An example of account modification is:

    \qml
        import org.nemomobile.accounts 1.0

        Item {
            id: root
            
            Account {
                id: account
                identifier: 12 // retrieved from AccountManager or AccountModel

                // we will be updating the following two properties
                displayName: "inactive example account"
                enabled: false

                onStatusChanged: {
                    if (status == Account.Initialized) {
                        sync() // trigger database write
                    } else if (status == Account.Error) {
                        // handle error
                    } else if (status == Account.Synced) {
                        // successfully written to database
                        // for example purposes, we may want to remove the account.
                        remove() // trigger database write
                    } else if (status == Account.Invalid) {
                        // successfully removed from database.
                    }
                }
            }
        }
    \endqml

    An Account can be associated with an Identity via the Identity's
    id.  This will allow clients to use a ServiceAccountIdentity
    derived from this Account to sign on to a service (using the
    credentials encapsulated in the Identity).  See the documentation
    about the Identity type from the SignOn adapters module for more
    information about this.
*/

AccountInterface::AccountInterface(QObject *parent)
    : QObject(parent), d(new AccountInterfacePrivate(this, 0))
{
}

AccountInterface::~AccountInterface()
{
}


// QDeclarativeParserStatus
void AccountInterface::classBegin() { }
void AccountInterface::componentComplete()
{
    if (!d->account) {
        if (d->identifier == 0) {
            if (d->providerName.isEmpty()) {
                // no id and no provider - can't create account.
                d->setStatus(AccountInterface::Invalid); // Set to invalid even though already set to error!  Since no account.
            } else {
                // create a new account
                Accounts::Account *newAccount = d->manager->createAccount(d->providerName);
                d->setAccount(newAccount);
                setEnabled(true); // enable by default.
            }
        } else {
            if (d->providerName.isEmpty()) {
                // loading an existing account
                Accounts::Account *existingAccount = d->manager->account(d->identifier);
                d->setAccount(existingAccount);
            } else {
                // provided both an id and a provider name... error.
                d->setStatus(AccountInterface::Invalid); // Set to invalid even though already set to error!  Since no account.
            }
        }
    } else {
        // account was provided by IdentityManager.
        // do nothing.
    }
}

// helpers for AccountManagerInterface only.
AccountInterface::AccountInterface(Accounts::Account *account, QObject *parent)
    : QObject(parent), d(new AccountInterfacePrivate(this, account)) { }
Accounts::Account *AccountInterface::account() { return d->account; }

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
    if (d->status == AccountInterface::Initializing)
        d->pendingSync = true;

    if (d->status == AccountInterface::Invalid
            || d->status == AccountInterface::SyncInProgress
            || d->status == AccountInterface::Initializing)
        return;

    if (!d->account) { // initialization failed.
        d->error = AccountInterface::InitializationFailedError;
        emit errorChanged();
        d->setStatus(AccountInterface::Invalid);
        return;
    }

    if (d->pendingInitModifications) {
        // we have handled them by directly syncing.
        // after this sync, we will once again allow
        // change signals to cause modifications to the properties.
        d->pendingInitModifications = false;
        d->identifierPendingInit = false;
        d->providerNamePendingInit = false;
        d->identityIdentifiersPendingInit = false;
        d->enabledPendingInit = false;
        d->displayNamePendingInit = false;
        d->configurationValuesPendingInit = false;
        d->enabledServiceNamesPendingInit = false;
    }

    // set the credentials / identity identifiers for each service.
    foreach (const QString &srvn, d->supportedServiceNames) {
        int currIdentId = d->identityIdentifiers.value(srvn).value<int>();
        Accounts::Service srv = d->manager->service(srvn);
        if (srv.isValid()) {
            d->account->selectService(srv);
            d->account->setCredentialsId(currIdentId);
            d->account->selectService(Accounts::Service());
        }
    }

    // set the enabled services correctly.
    foreach (const QString &srvn, d->supportedServiceNames) {
        Accounts::Service srv = d->manager->service(srvn);
        if (srv.isValid()) {
            d->account->selectService(srv);
            if (d->enabledServiceNames.contains(srvn))
                d->account->setEnabled(true);
            else
                d->account->setEnabled(false);
            d->account->selectService(Accounts::Service());
        }
    }

    // remove any which aren't part of the supported services set.
    QStringList tmpESN = d->enabledServiceNames;
    QStringList improvedESN;
    foreach (const QString &esn, tmpESN) {
        if (d->supportedServiceNames.contains(esn))
            improvedESN.append(esn);            
    }
    if (tmpESN != improvedESN) {
        d->enabledServiceNames = improvedESN;
        emit enabledServiceNamesChanged();
    }

    // set the configuration values.
    QStringList allKeys = d->account->allKeys();
    QStringList setKeys = d->configurationValues.keys();
    QStringList doneKeys;
    foreach (const QString &key, allKeys) {
        // overwrite existing keys
        if (setKeys.contains(key)) {
            doneKeys.append(key);
            const QVariant &currValue = d->configurationValues.value(key);
            if (currValue.isValid()) {
                d->account->setValue(key, currValue);
            } else {
                d->account->remove(key);
            }
        } else {
            // remove removed keys
            d->account->remove(key);
        }
    }
    foreach (const QString &key, setKeys) {
        // add new keys
        if (!doneKeys.contains(key)) {
            const QVariant &currValue = d->configurationValues.value(key);
            d->account->setValue(key, currValue);
        }
    }

    d->account->setEnabled(d->enabled);
    d->account->setDisplayName(d->displayName);

    d->setStatus(AccountInterface::SyncInProgress);
    d->account->sync();
}

/*!
    \qmlmethod void Account::remove()

    Removes the account.  A removed account becomes invalid.
*/
void AccountInterface::remove()
{
    if (!d->account)
        return;

    d->setStatus(AccountInterface::SyncInProgress);
    d->account->remove();
    d->account->sync();
}

/*!
    \qmlmethod void Account::setConfigurationValue(const QString &key, const QVariant &value)
    Sets the account's configuration settings value for the key \a key
    to the value \a value.  The only supported value types are int, quint64, bool and QString.
*/
void AccountInterface::setConfigurationValue(const QString &key, const QVariant &value)
{
    if (d->status == AccountInterface::Invalid || d->status == AccountInterface::SyncInProgress)
        return;

    if (value.type() != QVariant::Int
            && value.type() != QVariant::LongLong
            && value.type() != QVariant::ULongLong
            && value.type() != QVariant::Bool
            && value.type() != QVariant::String) {
        qWarning() << Q_FUNC_INFO << "Unsupported configuration value type!  Must be int, quint64, bool or string.";
        return; // unsupported value type.
    }

    d->configurationValues.insert(key, value);
    if (d->status == AccountInterface::Initializing)
        d->configurationValuesPendingInit = true;
    else
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
    if (d->status == AccountInterface::Invalid || d->status == AccountInterface::SyncInProgress)
        return;

    d->configurationValues.remove(key);
    if (d->status == AccountInterface::Initializing)
        d->configurationValuesPendingInit = true;
    else
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
    if (d->status == AccountInterface::Invalid || d->status == AccountInterface::SyncInProgress)
        return;

    if (!d->enabledServiceNames.contains(serviceName)) {
        d->enabledServiceNames.append(serviceName);
        if (d->status == AccountInterface::Initializing)
            d->enabledServiceNamesPendingInit = true;
        else
            d->setStatus(AccountInterface::Modified);
        // we don't emit enabledServiceNamesChanged here; we re-emit the sigs after sync()
    }
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
    if (d->status == AccountInterface::Invalid || d->status == AccountInterface::SyncInProgress)
        return;

    if (d->enabledServiceNames.contains(serviceName)) {
        d->enabledServiceNames.removeAll(serviceName);  
        if (d->status == AccountInterface::Initializing)
            d->enabledServiceNamesPendingInit = true;
        else
            d->setStatus(AccountInterface::Modified);
        // we don't emit enabledServiceNamesChanged here; we re-emit the sigs after sync()
    }
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
    if (d->status == AccountInterface::Invalid)
        return false;
    return d->enabled;
}

void AccountInterface::setEnabled(bool e)
{
    if (d->status == AccountInterface::Invalid || d->status == AccountInterface::SyncInProgress)
        return;

    d->enabled = e;
    if (d->status == AccountInterface::Initializing)
        d->enabledPendingInit = true;
    else
        d->setStatus(AccountInterface::Modified);
    emit enabledChanged();
}

/*!
    \qmlproperty int Account::identifier
    This property contains the identifier of the Account.

    The value of the property will be zero if the Account is a new, unsynced
    account.  If the Account has been saved in the system accounts database,
    it will be non-zero.

    When declaring an Account you may supply an identifier to cause
    the account to reference an account that already exists in the
    system accounts database.  Otherwise, you must supply a \c providerName
    to allow the Account to reference a new, unsaved account with the
    specified provider.

    Specifying both \c identifier and \c providerName in the Account
    declaration is an error.

    You cannot set the identifier of an Account after it has been initialized.
*/

int AccountInterface::identifier() const
{
    if (d->status == AccountInterface::Invalid)
        return 0;
    return d->identifier;
}

void AccountInterface::setIdentifier(int id)
{
    if (d->status == AccountInterface::Initializing) {
        if (d->providerName.isEmpty()) {
            d->identifierPendingInit = true;
            d->identifier = id;
        } else {
            d->error = AccountInterface::ConflictingProviderError;
            d->setStatus(AccountInterface::Error);
        }
    }
}

/*!
    \qmlproperty QVariantMap Account::identityIdentifiers

    The property contains the identifiers of each Identity (signon credentials)
    associated with the account, mapped to the services for which they are valid.
    The account-global credentials are mapped to an empty service name.
*/

QVariantMap AccountInterface::identityIdentifiers() const
{
    if (d->status == AccountInterface::Invalid)
        return QVariantMap();
    return d->identityIdentifiers;
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
    if (d->status == AccountInterface::Invalid)
        return 0;
    return d->identityIdentifiers.value(serviceName).value<int>();
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
    if (d->status == AccountInterface::Invalid || d->status == AccountInterface::SyncInProgress)
        return;

    d->identityIdentifiers.insert(serviceName, id);
    if (d->status == AccountInterface::Initializing)
        d->identityIdentifiersPendingInit = true;
    else
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

    A valid provider name must be specified in the Account declaration
    if no \c identifier is specified, so that the Account can reference
    a new, unsaved account with the specified provider.

    Specifying both \c identifier and \c providerName in the Account
    declaration is an error.

    You cannot set the \c providerName of an Account after it has been initialized.
*/

QString AccountInterface::providerName() const
{
    if (d->status == AccountInterface::Invalid)
        return QString();
    return d->providerName;
}

void AccountInterface::setProviderName(const QString &providerName)
{
    if (d->status == AccountInterface::Initializing) {
        if (d->identifier == 0) {
            d->providerNamePendingInit = true;
            d->providerName = providerName;
        } else if (!providerName.isEmpty()) {
            d->error = AccountInterface::ConflictingProviderError;
            d->setStatus(AccountInterface::Error);
        }
    }
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
    if (d->status == AccountInterface::Invalid)
        return QString();
    return d->displayName;
}

void AccountInterface::setDisplayName(const QString &dn)
{
    if (d->status == AccountInterface::Invalid || d->status == AccountInterface::SyncInProgress)
        return;

    d->displayName = dn;
    if (d->status == AccountInterface::Initializing)
        d->displayNamePendingInit = true;
    else
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
    if (d->status == AccountInterface::Invalid)
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
    if (d->status == AccountInterface::Invalid)
        return QStringList();
    return d->enabledServiceNames;
}



/*!
    \qmlproperty QVariantMap Account::configurationValues
    This property contains the configuration settings of the account

    Some default settings are usually specified in the \c{.service}
    file installed by the account provider plugin.  Other settings
    may be specified directly for an account.

    The only supported value types are int, quint64, bool and QString.
*/

QVariantMap AccountInterface::configurationValues() const
{
    if (d->status == AccountInterface::Invalid)
        return QVariantMap();
    return d->configurationValues;
}

void AccountInterface::setConfigurationValues(const QVariantMap &values)
{
    if (d->status == AccountInterface::Invalid || d->status == AccountInterface::SyncInProgress)
        return;

    QVariantMap validValues;
    QStringList vkeys = values.keys();
    foreach (const QString &key, vkeys) {
        QVariant currValue = values.value(key);
        if (currValue.type() == QVariant::Bool
                || currValue.type() == QVariant::Int
                || currValue.type() == QVariant::LongLong
                || currValue.type() == QVariant::ULongLong
                || currValue.type() == QVariant::String) {
            validValues.insert(key, currValue);
        }
    }

    if (d->configurationValues == validValues)
        return;

    d->configurationValues = validValues;
    if (d->status == AccountInterface::Initializing)
        d->configurationValuesPendingInit = true;
    else
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
