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

#include "identityinterface.h"
#include "identityinterface_p.h"

#include <QtDebug>

//libsignon-qt
#include <SignOn/Identity>
#include <SignOn/identityinfo.h>

IdentityInterfacePrivate::IdentityInterfacePrivate(SignOn::Identity *ident, IdentityInterface *parent)
    : QObject(parent), q(parent), identity(ident), initializationComplete(false),
      status(IdentityInterface::RefreshInProgress), error(IdentityInterface::NoError)
{
    if (ident)
        setIdentity(ident);
}

IdentityInterfacePrivate::~IdentityInterfacePrivate()
{
}

void IdentityInterfacePrivate::setIdentity(SignOn::Identity *ident) // should ONLY be done ONCE: either ctor OR from II::setIdentity().
{
    if (!ident) {
        qWarning() << "IdentityInterface::setIdentity() called with null identity! Aborting operation.";
        return;
    }

    identity = ident;
    identifier = identity->id();

    connect(identity, SIGNAL(info(SignOn::IdentityInfo)), this, SLOT(handleInfo(SignOn::IdentityInfo)));
    connect(identity, SIGNAL(removed()), this, SLOT(handleRemoved()));
    connect(identity, SIGNAL(error(SignOn::Error)), this, SLOT(handleError(SignOn::Error)));
    connect(identity, SIGNAL(credentialsStored(quint32)), this, SLOT(handleCredentialsStored(quint32)));

    // first time read from db.  we should be in RefreshInProgress state to begin with.
    // QueuedConnection to ensure that clients have a chance to connect to state changed signals
    // of the Identity object returned from IdentityManager::createIdentity().
    QMetaObject::invokeMethod(this, "asyncQueryInfo", Qt::QueuedConnection);
}

void IdentityInterfacePrivate::asyncQueryInfo()
{
    identity->queryInfo();
}

void IdentityInterfacePrivate::handleInfo(const SignOn::IdentityInfo &retInfo)
{
    if (status == IdentityInterface::Invalid)
        return;

    // refresh completed.
    // Overwrite all existing info property values, but only
    // emit change signals where required.
    info = retInfo;

    int currentIdentifier = identity->id();
    if (identifier != currentIdentifier) {
        identifier = currentIdentifier;
        emit q->identifierChanged();
    }

    if (userName != info.userName()) {
        userName == info.userName();
        emit q->userNameChanged();
    }

    // Cannot read the secret.
    // they either supply it (and have write privs)
    // or need to ask the user.

    if (caption != info.caption()) {
        caption = info.caption();
        emit q->captionChanged();
    }

    if (realms != info.realms()) {
        realms = info.realms();
        emit q->realmsChanged();
    }

    if (owner != info.owner()) {
        owner = owner;
        emit q->ownerChanged();
    }

    if (accessControlList != info.accessControlList()) {
        accessControlList = info.accessControlList();
        emit q->accessControlListChanged();
    }

    QMap<QString, QStringList> infoMethodMechs;
    QStringList infoMeths = info.methods();
    foreach (const QString &method, infoMeths)
        infoMethodMechs.insert(method, info.mechanisms(method));
    if (methodMechanisms != infoMethodMechs) {
        methodMechanisms = infoMethodMechs;
        emit q->methodsChanged();
    }

    if (!initializationComplete) {
        initializationComplete = true;
        setStatus(IdentityInterface::Initialized); // first time update complete.
    } else {
        setStatus(IdentityInterface::Synced); // we're up to date now.
    }
}

void IdentityInterfacePrivate::handleCredentialsStored(quint32 newId)
{
    if (status == IdentityInterface::Invalid)
        return;

    int currentIdentifier = newId;
    if (identifier != currentIdentifier) {
        identifier = currentIdentifier;
        emit q->identifierChanged();
    }

    setStatus(IdentityInterface::Synced);
}

void IdentityInterfacePrivate::handleRemoved()
{
    setStatus(IdentityInterface::Invalid);
}

void IdentityInterfacePrivate::handleError(SignOn::Error err)
{
    error = static_cast<IdentityInterface::ErrorType>(err.type());
    errorMessage = err.message();
    setStatus(IdentityInterface::Error);
    emit q->errorChanged();
    emit q->errorMessageChanged();
}

void IdentityInterfacePrivate::setStatus(IdentityInterface::Status newStatus)
{
    if (status == IdentityInterface::Invalid)
        return; // never update state from invalid to anything else.

    if (status != newStatus) {
        status = newStatus;
        emit q->statusChanged();
    }
}

//---------------------------

/*!
    \qmltype Identity
    \instantiates IdentityInterface
    \inqmlmodule org.nemomobile.signon 1
    \brief An Identity which contains credentials used for service signon

    This type is intended for use by privileged applications
    and account plugin providers.  It allows service signon
    credentials to be specified.  This type is not directly
    creatable; instead, instances should be retrieved from the
    IdentityManager either by calling createIdentity() or
    identity() for new or existing identities respectively.

    Once an identity has been created, it is usual to associate
    an account with the identity, in order to allow client
    applications to use that account to signon to the services
    for which the credentials in the identiy are valid.

    Example of usage:

    \qml
    import QtQuick 1.1
    import org.nemomobile.accounts 1.0
    import org.nemomobile.signon 1.0

    Item {
        id: root

        property AccountManager _acm: AccountManager {}
        property IdentityManager _idm: IdentityManager {}
        property QtObject _identity
        property QtObject _account

        // the following 3 things would be populated from the UI.
        property string userName: "username"
        property string secret: "password"
        property string caption: "caption"

        function addIdentityAndAccount() {
            console.log("Creating and storing identity...")
            _identity = _idm.createIdentity()
            _identity.statusChanged.connect(handleIdentityStatusChange)
        }

        function handleIdentityStatusChange() {
            if (_identity.status === Identity.Initialized) {
                // the identity is ready to be modified.
                _identity.userName = root.userName
                _identity.secret = root.secret
                _identity.caption = root.caption
                _identity.setMethodMechanisms("password", ["ClientLogin"])
                _identity.sync() // store to db
            } else if (_identity.status === Identity.Synced) {
                // success.  Create the account, store the credentials.
                console.log("Successfully created identity.  Creating and storing account...")
                _account = _acm.createAccount("google")
                _account.statusChanged.connect(handleAccountStatusChange)
                _account.displayName = "example account"
                _account.enabled = true
                _account.enableAccountWithService("google-talk")
                _account.setIdentityIdentifier(_identity.identifier, "google-talk")
                _account.sync() // store to db
            } else if (_identity.status === Identity.Invalid || _identity.status === Identity.SyncError) {
                // Identity was removed or error occurred; display error and exit
                console.log("Error storing identity: " + _identity.error + ": " + _identity.errorMessage)
            }
        }

        function handleAccountStatusChange() {
            if (_account.status === Account.Synced) {
                // Finished syncing, display success message
                console.log("Successfully created account: " + _account.displayName + " with credentials id: " + _account.identityIdentifier)
            } else if (_account.status === Account.Invalid || _account.status === Account.SyncError) {
                // Account was removed or error occured; display error and exit
                console.log("Error storing account: " + _account.error + ": " + _account.errorMessage)
            }
        }
    }
    \endqml
*/

IdentityInterface::IdentityInterface(SignOn::Identity *ident, QObject *parent)
    : QObject(parent), d(new IdentityInterfacePrivate(ident, this))
{
}

IdentityInterface::~IdentityInterface()
{
}

/*!
    \qmlproperty int Identity::identifier
    Contains the identifier of the identity
*/

int IdentityInterface::identifier() const
{
    return d->identifier;
}

/*!
    \qmlproperty IdentityInterface::Status Identity::status
    Contains the status of the identity
*/

IdentityInterface::Status IdentityInterface::status() const
{
    return d->status;
}

/*!
    \qmlproperty IdentityInterface::Error Identity::error
    Contains the most recent error which occurred during
    creation or synchronisation of the identity.  Note that
    this property will NOT be updated to NoError if subsequent
    synchronisation operations succeed.
*/
    

IdentityInterface::ErrorType IdentityInterface::error() const
{
    return d->error;
}

/*!
    \qmlproperty string Identity::errorMessage
    Contains the message associated with the most recent
    error which occurred during creation or synchronisation
    of the identity.  Note that this property will NOT
    be updated to NoError if subsequent synchronisation
    operations succeed.
*/

QString IdentityInterface::errorMessage() const
{
    return d->errorMessage;
}

/*!
    \qmlproperty string Identity::userName
    Contains the user name part of the credentials which may be
    used to sign in to services associated with this identity.
*/

QString IdentityInterface::userName() const
{
    return d->userName;
}

void IdentityInterface::setUserName(const QString &uname)
{
    if (d->status != IdentityInterface::Invalid
            && d->status != IdentityInterface::SyncInProgress
            && d->status != IdentityInterface::RefreshInProgress
            && d->userName != uname) {
        d->userName = uname;
        d->setStatus(IdentityInterface::Modified);
        emit userNameChanged();
    }
}

/*!
    \qmlproperty string Identity::secret
    Contains the secret part of the credentials which may be
    used to sign in to services associated with this Identity.
    Note that the secret cannot be read from the database, and
    thus must either be provided when creating the identity,
    or will be empty.

    When calling sync() to update the database, you must explicitly
    define the mode with which the synchronisation will occur.
    One of the modes causes the value of the secret associated with
    this identity in the database to be overwritten, whilst the
    other discards the Identity's secret when synchronising.
*/

QString IdentityInterface::secret() const
{
    return d->secret;
}

void IdentityInterface::setSecret(const QString &sec)
{
    if (d->status != IdentityInterface::Invalid
            && d->status != IdentityInterface::SyncInProgress
            && d->status != IdentityInterface::RefreshInProgress
            && d->secret != sec) {
        d->secret = sec;
        d->setStatus(IdentityInterface::Modified);
        emit secretChanged();
    }
}

/*!
    \qmlproperty string Identity::caption
    Contains the caption part of the credentials which may be
    used to sign in to services associated with this Identity.
    The caption may be required to determine whether the user
    is likely to be a human or not.
*/

QString IdentityInterface::caption() const
{
    return d->caption;
}

void IdentityInterface::setCaption(const QString &cap)
{
    if (d->status != IdentityInterface::Invalid
            && d->status != IdentityInterface::SyncInProgress
            && d->status != IdentityInterface::RefreshInProgress
            && d->caption != cap) {
        d->caption = cap;
        d->setStatus(IdentityInterface::Modified);
        emit captionChanged();
    }
}

/*!
    \qmlproperty QStringList Identity::realms
    Contains the realms (or domains) for which the credentials
    encapsulated by this Identity are valid.  For example,
    several different online services provided by the same provider
    may all be accessible using the same credentials.
*/

QStringList IdentityInterface::realms() const
{
    return d->realms;
}

void IdentityInterface::setRealms(const QStringList &rlms)
{
    if (d->status != IdentityInterface::Invalid
            && d->status != IdentityInterface::SyncInProgress
            && d->status != IdentityInterface::RefreshInProgress
            && d->realms != rlms) {
        d->realms = rlms;
        d->setStatus(IdentityInterface::Modified);
        emit realmsChanged();
    }
}

/*!
    \qmlproperty string Identity::owner
    Contains the owner of the Identity
*/

QString IdentityInterface::owner() const
{
    return d->owner;
}

void IdentityInterface::setOwner(const QString &own)
{
    if (d->status != IdentityInterface::Invalid
            && d->status != IdentityInterface::SyncInProgress
            && d->status != IdentityInterface::RefreshInProgress
            && d->owner != own) {
        d->owner = own;
        d->setStatus(IdentityInterface::Modified);
        emit ownerChanged();
    }
}

/*!
    \qmlproperty QStringList Identity::accessControlList
    Contains the access control list associated with the credentials
    encapsulated by this Identity.
*/

QStringList IdentityInterface::accessControlList() const
{
    return d->accessControlList;
}

void IdentityInterface::setAccessControlList(const QStringList& acl)
{
    if (d->status != IdentityInterface::Invalid
            && d->status != IdentityInterface::SyncInProgress
            && d->status != IdentityInterface::RefreshInProgress
            && d->accessControlList != acl) {
        d->accessControlList = acl;
        d->setStatus(IdentityInterface::Modified);
        emit accessControlListChanged();
    }
}



//refCount and type are uncommon; lets not include them in our API for now.
///*!
//    \qmlproperty IdentityInterface::CredentialsType Identity::type
//    Contains the type of the credentials which are encapsulated by
//    this Identity.
//*/
//IdentityInterface::CredentialsType IdentityInterface::type() const
//{
//    return d->type;
//}
//
//void IdentityInterface::setType(CredentialsType t)
//{
//    if (d->status != IdentityInterface::Invalid
//            && d->status != IdentityInterface::SyncInProgress
//            && d->status != IdentityInterface::RefreshInProgress
//            && d->type != t) {
//        d->type = t;
//        d->setStatus(IdentityInterface::Modified);
//        emit typeChanged();
//    }
//}
//
//int IdentityInterface::refCount() const
//{
//    return d->refCount;
//}
//
//void IdentityInterface::setRefCount(int c)
//{
//    if (d->status != IdentityInterface::Invalid
//            && d->status != IdentityInterface::SyncInProgress
//            && d->status != IdentityInterface::RefreshInProgress
//            && d->refCount != c) {
//        d->refCount = c;
//        d->setStatus(IdentityInterface::Modified);
//        emit refCountChanged();
//    }
//}

/*!
    \qmlproperty QStringList Identity::methods
    Contains the names of methods which are supported as
    authentication methods by the credentials encapsulated
    by this Identity.  Different services may require different
    methods to be used during authentication.
*/

QStringList IdentityInterface::methods() const
{
    return d->methodMechanisms.keys();
}


/*!
    \qmlmethod void Identity::setMethodMechanisms(const QString &methodName, const QStringList &mechanisms)
    Sets the mechanisms for the method identified by the given \a methodName to
    \a mechanisms.  Different services may require different mechanisms to be
    used during authentication, even if they use the same method.
*/
void IdentityInterface::setMethodMechanisms(const QString &methodName, const QStringList &mechanisms)
{
    if (d->status != IdentityInterface::Invalid
            && d->status != IdentityInterface::SyncInProgress
            && d->status != IdentityInterface::RefreshInProgress) {
        if (!d->methodMechanisms.contains(methodName)
                || d->methodMechanisms.value(methodName) != mechanisms) {
            d->methodMechanisms.insert(methodName, mechanisms);
            d->setStatus(IdentityInterface::Modified);
            emit methodsChanged();
        }
    }
}

/*!
    \qmlmethod QStringList Identity::methodMechanisms(const QString &methodName)
    Returns the mechanisms associated with the method identified by
    the given \a methodName
*/
QStringList IdentityInterface::methodMechanisms(const QString &methodName)
{
    return d->methodMechanisms.value(methodName);
}

/*!
    \qmlmethod void Identity::removeMethod(const QString &methodName)
    Removes the method (and any associated mechanisms) from the list of
    methods supported by the credentials encapsulated by this Identity.
*/
void IdentityInterface::removeMethod(const QString &methodName)
{
    d->methodMechanisms.remove(methodName);
    d->setStatus(IdentityInterface::Modified);
    emit methodsChanged();
}

/*!
    \qmlmethod void Identity::sync(Identity::SyncMode mode)

    May only be called if the current status is Modified.
    Calling sync() if the status has any other value, will have no effect.
    After calling sync(), the status will change to SyncInProgress,
    and a (possibly, but not certainly, asynchronous) database write will occur.
    Once the data has been written to the database (or an error has occurred),
    the status will change to Synced (or Error).

    You can not make any changes to properties whilst in the SyncInProgress
    state.

    If the \a mode parameter is Identity::OverwriteSecret, the secret of the underlying
    identity will be overwitten by the sync() process, otherwise if the mode is
    Identity::NoOverwriteSecret the database  version of the secret will be left
    intact.
*/
void IdentityInterface::sync(IdentityInterface::SyncMode mode)
{
    if (d->status != IdentityInterface::Modified)
        return;

    if (mode == IdentityInterface::OverwriteSecret) {
        d->info.setSecret(d->secret);
        d->info.setStoreSecret(true);
    } else if (mode == IdentityInterface::NoOverwriteSecret) {
        d->info.setStoreSecret(false);
    }

    d->info.setUserName(d->userName);
    d->info.setCaption(d->caption);
    d->info.setRealms(d->realms);
    d->info.setOwner(d->owner);
    d->info.setAccessControlList(d->accessControlList);
    QStringList existingMethods = d->info.methods();
    foreach (const QString &meth, existingMethods)
        d->info.removeMethod(meth);
    QStringList newMethods = d->methodMechanisms.keys();
    foreach (const QString &method, newMethods)
        d->info.setMethod(method, d->methodMechanisms.value(method));

    d->setStatus(IdentityInterface::SyncInProgress);
    d->identity->storeCredentials(d->info); // the docs are a bit unclear.. I assume this does what I think it does.
}

/*!
    \qmlmethod void Identity::refresh()

    May only be called if the current status is Initialized, Synced, Modified or Error.
    Calling refresh() if status has any other value, will have no effect.
    After calling refresh(), the status will change to RefreshInProgress,
    and a (possibly, but not certainly, asynchronous) database read will occur.
    Once the data has been retrieved from the database (or an error has occurred),
    the status will change to Synced and the property values will be updated to the
    newly retrieved database state (or the status will change to Error).

    Note that if an error occurred during creation of the identity, the first
    time refresh() succeeds, the state will transition to Initialized rather
    than Synced.

    Note that the secret cannot be refreshed; if you have access to write it,
    you may attempt to do so, but it cannot be read back.

    Any changes you have made to any properties will be overwritten when the
    refresh completes.
*/
void IdentityInterface::refresh()
{
    if (d->status != IdentityInterface::Synced
            && d->status != IdentityInterface::Initialized
            && d->status != IdentityInterface::Modified
            && d->status != IdentityInterface::Error)
        return;

    d->setStatus(IdentityInterface::RefreshInProgress);
    d->identity->queryInfo();
}

// the following functions are helpers for the IdentityManagerInterface ONLY
void IdentityInterface::setIdentity(SignOn::Identity *ident)
{
    if (d->identity) {
        qWarning() << "IdentityInterface::setIdentity() Warning! Already have identity set! Aborting operation.";
        return;
    }

    d->setIdentity(ident);
}

SignOn::Identity *IdentityInterface::identity() const
{
    return d->identity;
}

