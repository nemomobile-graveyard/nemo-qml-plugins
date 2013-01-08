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

#include "sessiondatainterface.h"

#include <QtDebug>

//libsignon-qt
#include <SignOn/Identity>
#include <SignOn/AuthSession>

ServiceAccountIdentityInterfacePrivate::ServiceAccountIdentityInterfacePrivate(SignOn::Identity *ident, ServiceAccountIdentityInterface *parent)
    : QObject(parent), q(parent), session(0), identity(ident)
    , startedInitialization(false)
    , finishedInitialization(false)
    , status(ServiceAccountIdentityInterface::Invalid)
    , error(ServiceAccountIdentityInterface::NoError)
{
    if (ident) {
        ownIdentity = false;
        startedInitialization = true;
    } else {
        ownIdentity = true;
    }
    setIdentity(ident, false, false);
}

ServiceAccountIdentityInterfacePrivate::~ServiceAccountIdentityInterfacePrivate()
{
    if (ownIdentity)
        delete identity; // will destroy any sessions, as the identity is the owner of all created sessions.
    else if (session && identity)
        identity->destroySession(session); // destroy session manually.
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
        connect(identity, SIGNAL(info(SignOn::IdentityInfo)), this, SLOT(handleInfo(SignOn::IdentityInfo)));

        // clear our state.
        status = ServiceAccountIdentityInterface::Initializing; // manual set, instead of via setState() which filters if invalid.
        error = ServiceAccountIdentityInterface::NoError;
        errorMessage = QString();
        methodMechanisms = QMap<QString, QStringList>();
        currentMethod = QString();
        currentMechanism = QString();

        // trigger refresh
        identity->queryInfo();

        if (emitSignals) { // don't want to emit during ctor.
            emit q->statusChanged();
            emit q->errorChanged();
            emit q->errorMessageChanged();
            emit q->methodsChanged();
        }
    } else {
        setStatus(ServiceAccountIdentityInterface::Invalid); // not valid if 0.
    }
}

void ServiceAccountIdentityInterfacePrivate::setStatus(ServiceAccountIdentityInterface::Status newStatus, const QString &message)
{
    if (status == ServiceAccountIdentityInterface::Invalid)
        return;

    bool sne = (status != newStatus);
    bool smne = (statusMessage != message);
    if (sne)
        status = newStatus;
    if (smne)
        statusMessage = message;
    if (sne)
        emit q->statusChanged();
    if (smne)
        emit q->statusMessageChanged();
}

void ServiceAccountIdentityInterfacePrivate::handleError(const SignOn::Error &err)
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

void ServiceAccountIdentityInterfacePrivate::handleInfo(const SignOn::IdentityInfo &info)
{
    // in ServiceAccountIdentityInterface we only use info to get the methods + mechanisms.
    // XXX TODO: only populate with the methods/mechanisms applicable to the service.
    QMap<QString, QStringList> infoMethodMechs;
    QStringList infoMeths = info.methods();
    foreach (const QString &method, infoMeths)
        infoMethodMechs.insert(method, info.mechanisms(method));
    if (methodMechanisms != infoMethodMechs) {
        methodMechanisms = infoMethodMechs;
        emit q->methodsChanged();
    }

    finishedInitialization = true;
    if (status == ServiceAccountIdentityInterface::Initializing)
        setStatus(ServiceAccountIdentityInterface::Initialized); // don't overwrite if error.
}

// AuthSession response
void ServiceAccountIdentityInterfacePrivate::handleResponse(const SignOn::SessionData &sd)
{
    // XXX TODO: should we pass back a SessionDataInterface ptr instead?  Ownership concerns?
    QVariantMap dataMap;
    QStringList keys = sd.propertyNames();
    foreach (const QString &key, keys)
        dataMap.insert(key, sd.getProperty(key));
    emit q->responseReceived(dataMap);
}

// AuthSession status
void ServiceAccountIdentityInterfacePrivate::handleStateChanged(SignOn::AuthSession::AuthSessionState newState, const QString &message)
{
    setStatus(static_cast<ServiceAccountIdentityInterface::Status>(newState), message);
}

void ServiceAccountIdentityInterfacePrivate::setUpSessionSignals()
{
    connect(session, SIGNAL(error(SignOn::Error)), this, SLOT(handleError(SignOn::Error)));
    connect(session, SIGNAL(response(SignOn::SessionData)), this, SLOT(handleResponse(SignOn::SessionData)));
    connect(session, SIGNAL(stateChanged(AuthSession::AuthSessionState, QString)),
            this, SLOT(handleStateChanged(AuthSession::AuthSessionState, QString)));
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
        property variant sessionData // retrieved from a ServiceAccount (authData.parameters)

        ServiceAccountIdentity {
            id: ident
            identifier: serviceAccountIdentityIdentifier

            onResponseReceived: {
                // enumerate signon tokens from the returned session data, etc.
                // then continue sign on with updated session data, or perform API calls, etc.
                sessionData = {"Auth": data["Auth"]}
                process(sessionData)
            }

            onStatusChanged: {
                if (status == ServiceAccountIdentity.Initialized) {
                    signIn("password", "ClientLogin", sessionData)
                } else if (status == ServiceAccountIdentity.Error) {
                    console.log("Error (" + error + ") occurred during authentication: " + errorMessage)
                }
            }
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
    \qmlmethod bool ServiceAccountIdentity::signIn(const QString &method, const QString &mechanism, const QVariantMap &sessionData)

    Begins sign-in to the service via the specified \a method and
    \a mechanism, using the given \a sessionData parameters.
    Returns true if the session could be created.
    Once the sign-in request has been processed successfully, the
    responseReceived() signal will be emitted.  If sign-in occurs
    in multiple stages (for example, via token swapping), you must
    call process() to progress the sign-in operation.

    Only one sign-in session may exist for a ServiceAccountIdentity at any
    time.  To start a new sign-in session, you must signOut() of the
    current sign-in session, if one exists, prior to calling signIn().

    Usually only one single method and one single mechanism will be valid
    to sign in with, and these can be retrieved from the \c authData of the
    \c ServiceAccount with which this \c ServiceAccountIdentity is associated.
*/
bool ServiceAccountIdentityInterface::signIn(const QString &method, const QString &mechanism, const QVariantMap &sessionData)
{
    if (d->status == ServiceAccountIdentityInterface::Invalid
            || d->status == ServiceAccountIdentityInterface::Initializing
            || !d->finishedInitialization) {
        return false;
    }

    if (d->session) {
        qWarning() << Q_FUNC_INFO << "Sign-in requested while previous sign-in session exists!";
        return false; // not a continuation.  they need to sign out first.
    }

    // beginning new sign-on
    d->session = d->identity->createSession(method);

    if (!d->session) {
        qWarning() << Q_FUNC_INFO << "Failed to create sign-in session.";
        return false;
    }

    d->currentMethod = method;
    d->currentMechanism = mechanism;
    d->setUpSessionSignals();
    d->session->process(SignOn::SessionData(sessionData), mechanism);
    return true;
}

/*!
    \qmlmethod void ServiceAccountIdentity::process(const QVariantMap &sessionData)
    Processes the session request defined by the given \a sessionData in order
    to progress a multi-stage sign-in operation.
*/
void ServiceAccountIdentityInterface::process(const QVariantMap &sessionData)
{
    if (d->status == ServiceAccountIdentityInterface::Invalid)
        return;

    if (d->session)
        d->session->process(SignOn::SessionData(sessionData), d->currentMechanism);
}

/*!
    \qmlmethod void ServiceAccountIdentity::signOut()
    Signs out of the service, if a sign-in process has been initiated.
    The status of the ServiceAccountIdentity will be set to \c NotStarted.
*/
void ServiceAccountIdentityInterface::signOut()
{
    if (d->status == ServiceAccountIdentityInterface::Invalid)
        return;

    if (d->session) {
        SignOn::AuthSession *temp = d->session;
        d->session = 0;
        d->currentMethod = QString();
        d->currentMechanism = QString();
        d->identity->destroySession(temp);
        d->setStatus(ServiceAccountIdentityInterface::NotStarted);
    }
}

/*!
    \qmlproperty int ServiceAccountIdentity::identifier
    Contains the identifier of the identity.

    The ServiceAccountIdentity will have a status of Invalid
    until its identifier is set to the identifier of a valid
    Identity in the database.

    Once set, you may not change the identifier.
*/

void ServiceAccountIdentityInterface::setIdentifier(int identityId)
{
    // XXX TODO: make this more robust using QDeclarativeParserStatus / Initializing.
    if (identityId == 0)
        return;

    if (!d->startedInitialization) {
        d->startedInitialization = true;
        SignOn::Identity *ident = SignOn::Identity::existingIdentity(identityId, d);
        d->setIdentity(ident, true, true);
        emit identifierChanged();
    }
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
    \qmlproperty QString ServiceAccountIdentity::statusMessage
    Contains the message associated with the status of the service account identity,
    if an associated message exists.
*/

QString ServiceAccountIdentityInterface::statusMessage() const
{
    return d->statusMessage;
}

/*!
    \qmlproperty QStringList ServiceAccountIdentity::methods()
    Contains the list of methods which are available to be used to sign on to the service
    with this identity.
*/
QStringList ServiceAccountIdentityInterface::methods() const
{
    return d->methodMechanisms.keys();
}

/*!
    \qmlproperty QStringList ServiceAccountIdentity::methodMechanisms(const QString &methodName)
    Contains the list of mechanisms which are valid for the method with
    the given \a methodName.
*/
QStringList ServiceAccountIdentityInterface::methodMechanisms(const QString &methodName) const
{
    return d->methodMechanisms.value(methodName);
}

