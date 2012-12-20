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

#include "serviceaccountidentityinterface.h"
#include "serviceaccountidentityinterface_p.h"

#include "authsessioninterface.h"

#include <QtDebug>

//libsignon-qt
#include <SignOn/Identity>
#include <SignOn/AuthSession>


ServiceAccountIdentityInterfacePrivate::ServiceAccountIdentityInterfacePrivate(SignOn::Identity *ident, ServiceAccountIdentityInterface *parent)
    : QObject(parent), q(parent), identity(ident), finishedInitialization(false)
    , status(ServiceAccountIdentityInterface::Invalid)
    , error(ServiceAccountIdentityInterface::NoError)
{
    if (ident)
        ownIdentity = false;
    else
        ownIdentity = true;
    setIdentity(ident, false, false);
}

ServiceAccountIdentityInterfacePrivate::~ServiceAccountIdentityInterfacePrivate()
{
    if (ownIdentity)
        delete identity;
}

void ServiceAccountIdentityInterfacePrivate::setIdentity(SignOn::Identity *ident, bool emitSignals, bool takeOwnership)
{
    if (identity)
        disconnect(identity, 0, this, 0);

    if (ownIdentity)
        delete identity;

    if (takeOwnership)
        ownIdentity = true;

    identity = ident;

    if (ident) {
        connect(identity, SIGNAL(error(SignOn::Error)), this, SLOT(handleError(SignOn::Error)));
        connect(identity, SIGNAL(removed()), this, SLOT(handleRemoved()));
        connect(identity, SIGNAL(secretVerified(bool)), q, SIGNAL(secretVerified(bool)));
        connect(identity, SIGNAL(userVerified(bool)), q, SIGNAL(userVerified(bool)));
        connect(identity, SIGNAL(methodsAvailable(QStringList)), this, SLOT(handleMethodsAvailable(QStringList)));

        // clear our state.
        status = ServiceAccountIdentityInterface::RefreshInProgress; // manual set, instead of via setState() which filters if invalid.
        error = ServiceAccountIdentityInterface::NoError;
        errorMessage = QString();
        availableMethods = QStringList();

        // trigger refresh
        identity->queryAvailableMethods();

        if (emitSignals) { // don't want to emit during ctor.
            emit q->statusChanged();
            emit q->errorChanged();
            emit q->errorMessageChanged();
            emit q->availableMethodsChanged();
        }
    } else {
        setStatus(ServiceAccountIdentityInterface::Invalid); // not valid if 0.
    }
}

void ServiceAccountIdentityInterfacePrivate::setStatus(ServiceAccountIdentityInterface::Status newStatus)
{
    if (status == ServiceAccountIdentityInterface::Invalid)
        return;

    if (status != newStatus) {
        status = newStatus;
        emit q->statusChanged();
    }
}

void ServiceAccountIdentityInterfacePrivate::handleError(SignOn::Error err)
{
    error = static_cast<ServiceAccountIdentityInterface::ErrorType>(err.type());
    errorMessage = err.message();
    setStatus(ServiceAccountIdentityInterface::Error);
    emit q->errorChanged();
    emit q->errorMessageChanged();
}

void ServiceAccountIdentityInterfacePrivate::handleRemoved()
{
    setStatus(ServiceAccountIdentityInterface::Invalid);
}

void ServiceAccountIdentityInterfacePrivate::handleMethodsAvailable(const QStringList &methods)
{
    availableMethods = methods;
    emit q->availableMethodsChanged();
    if (status == ServiceAccountIdentityInterface::RefreshInProgress) {
        if (!finishedInitialization) {
            finishedInitialization = true;
            setStatus(ServiceAccountIdentityInterface::Initialized);
        } else {
            setStatus(ServiceAccountIdentityInterface::Synced);
        }
    }
}

//--------------------------


/*!
    \qmltype ServiceAccountIdentity
    \instantiates ServiceAccountIdentityInterface
    \inqmlmodule org.nemomobile.signon 1
    \brief A ServiceAccountIdentity encapsulates the credentials associated with
           a particular account for a particular service.

    This type is intended for use by client applications.
    After retrieving a ServiceAccount, they may retrieve the id of the
    associated ServiceAccountIdentity from the ServiceAccount's
    AuthData property.  A client can then instantiate a ServiceAccountIdentity
    with this id, and use it to create authentication sessions with the
    service provider.

    Example of usage:

    \qml
    import QtQuick 1.1
    import org.nemomobile.signon 1.0
    import org.nemomobile.accounts 1.0

    Item {
        id: root

        property int serviceAccountIdentityIdentifier // retrieved from a ServiceAccount
        property QtObject sessionData // retrieved from a ServiceAccount (parameters)
        property QtObject session

        ServiceAccountIdentity {
            id: ident
            identifier: serviceAccountIdentityIdentifier
            onStatusChanged: {
                if (status == ServiceAccountIdentity.Error) {
                    console.log("Error occurred during authentication: " + error)
                }
            }
        }

        function signon() {
            if (ident.status == ServiceAccountIdentity.Initialized) {
                session = ident.createSession("password") // method = password
                session.response.connect(handleResponse)
                session.process(sessionData, "ClientLogin") // mechanism = ClientLogin, retrieved from ServiceAccount's AuthData
            }
        }

        function handleResponse(sessionData) {
            // enumerate signon tokens from the returned session data, etc.
        }
    }
    \endqml
*/

ServiceAccountIdentityInterface::ServiceAccountIdentityInterface(SignOn::Identity *ident, QObject *parent)
    : QObject(parent), d(new ServiceAccountIdentityInterfacePrivate(ident, this))
{
}

ServiceAccountIdentityInterface::~ServiceAccountIdentityInterface()
{
}

/*!
    \qmlmethod void ServiceAccountIdentity::refreshAvailableMehods()

    Triggers refresh of the available methods which may be used during signon
    with the identity and service encapsulated by the ServiceAccountIdentity.
    When refresh completes, the availableMethodsChanged() signal will be
    emitted even if no changes to the content of the availableMethods property
    occur.
*/
void ServiceAccountIdentityInterface::refreshAvailableMethods()
{
    if (d->status == ServiceAccountIdentityInterface::Invalid)
        return;
    d->identity->queryAvailableMethods();
}

/*!
    \qmlmethod void ServiceAccountIdentity::verifySecret(const QString &secret)

    Triggers verification of the given \a secret.
    The verification may occur synchronously or asynchronously.
    When verification completes, the secretVerified() signal
    will be emitted.
*/
void ServiceAccountIdentityInterface::verifySecret(const QString &secret)
{
    if (d->status == ServiceAccountIdentityInterface::Invalid)
        return;
    d->identity->verifySecret(secret);
}


/*!
    \qmlmethod void ServiceAccountIdentity::requestCredentialsUpdate(const QString &message)

    Triggers out-of-process dialogue creation which requests
    updated credentials from the user.

    The implementation of this function depends on the
    implementation of the underlying accountsd service.
*/
void ServiceAccountIdentityInterface::requestCredentialsUpdate(const QString &message)
{
    qWarning() << "ServiceAccountIdentityInterface::requestCredentialsUpdate() not implemented!";
    if (d->status == ServiceAccountIdentityInterface::Invalid)
        return;
    d->identity->requestCredentialsUpdate(message);
}

/*!
    \qmlmethod void ServiceAccountIdentity::verifyUser(const QString &message)
    Triggers out-of-process dialogue creation which attempts
    to verify the user, displaying the given \a message.

    The implementation of this function depends on the
    implementation of the underlying accountsd service.
*/
void ServiceAccountIdentityInterface::verifyUser(const QString &message)
{
    qWarning() << "ServiceAccountIdentityInterface::verifyUser() not implemented!";
    if (d->status == ServiceAccountIdentityInterface::Invalid)
        return;
    d->identity->verifyUser(message);
}

/*!
    \qmlmethod void ServiceAccountIdentity::verifyUser(const QVariantMap &params)

    Triggers out-of-process dialogue creation which attempts
    to verify the user, specifying the given \a params.

    The implementation of this function depends on the
    implementation of the underlying accountsd service.
*/
void ServiceAccountIdentityInterface::verifyUser(const QVariantMap &params)
{
    qWarning() << "ServiceAccountIdentityInterface::verifyUser() not implemented!";
    if (d->status == ServiceAccountIdentityInterface::Invalid)
        return;
    d->identity->verifyUser(params);
}

/*!
    \qmlmethod AuthSession* ServiceAccountIdentity::createSession(const QString &methodName, QObject *parent)

    Creates and returns an authentication session which uses the given \a methodName.
    The AuthSession returned will have the specified \a parent unless no
    parent is specified (in which case the ServiceAccountIdentity will own
    the AuthSession and delete it on destruction).
*/
AuthSessionInterface *ServiceAccountIdentityInterface::createSession(const QString &methodName, QObject *parent)
{
    if (d->status == ServiceAccountIdentityInterface::Invalid)
        return 0;

    SignOn::AuthSession *session = d->identity->createSession(methodName);
    if (!session)
        return 0;

    AuthSessionInterface *sif = 0;
    if (parent != 0)
        sif = new AuthSessionInterface(session, parent);
    else
        sif = new AuthSessionInterface(session, this);
    return sif;
}

/*!
    \qmlmethod void ServiceAccountIdentity::destroySession(AuthSession *session)

    Destroys the given AuthSession \a session.  This will sign the identity
    out of the service for which the AuthSession was created.
*/
void ServiceAccountIdentityInterface::destroySession(AuthSessionInterface *session)
{
    if (d->status == ServiceAccountIdentityInterface::Invalid)
        return;
    if (session)
        d->identity->destroySession(session->authSession());
}

/*!
    \qmlmethod void ServiceAccountIdentity::signOut()
    Signs the identity out of every service associated with this identity.
*/
void ServiceAccountIdentityInterface::signOut()
{
    if (d->status == ServiceAccountIdentityInterface::Invalid)
        return;
    d->identity->signOut();
}

/*!
    \qmlproperty int ServiceAccountIdentity::identifier
    Contains the identifier of the identity.

    The ServiceAccountIdentity will have a status of Invalid
    until its identifier is set to the identifier of a valid
    Identity in the database.
*/

void ServiceAccountIdentityInterface::setIdentifier(int identityId)
{
    SignOn::Identity *ident = SignOn::Identity::existingIdentity(identityId, d);
    d->setIdentity(ident, true, true);
    emit identifierChanged();
}

int ServiceAccountIdentityInterface::identifier() const
{
    if (d->status == ServiceAccountIdentityInterface::Invalid)
        return 0;
    return d->identity->id();
}

/*!
    \qmlproperty ServiceAccountIdentityInterface::ErrorType ServiceAccountIdentity::error
    Contains the most recent error which occurred during creation or signout.

    Note that this property will NOT automatically update to NoError if subsequent
    operations are successful.
*/

ServiceAccountIdentityInterface::ErrorType ServiceAccountIdentityInterface::error() const
{
    return d->error;
}

/*!
    \qmlproperty string ServiceAccountIdentity::errorMessage
    Contains the message associated with the most recent error which occurred during creation or signout.

    Note that this property will NOT automatically update to an empty string if subsequent
    operations are successful.
*/

QString ServiceAccountIdentityInterface::errorMessage() const
{
    return d->errorMessage;
}

/*!
    \qmlproperty ServiceAccountIdentityInterface::Status ServiceAccountIdentity::status
    Contains the status of the service account identity    
*/

ServiceAccountIdentityInterface::Status ServiceAccountIdentityInterface::status() const
{
    return d->status;
}

/*!
    \qmlproperty QStringList ServiceAccountIdentity::availableMethods()
    Contains the list of methods which are available to be used to create authentication sessions
*/
QStringList ServiceAccountIdentityInterface::availableMethods() const
{
    return d->availableMethods;
}

