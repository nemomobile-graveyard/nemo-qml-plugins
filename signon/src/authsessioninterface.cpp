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

#include "authsessioninterface.h"
#include "authsessioninterface_p.h"

#include "sessiondatainterface.h"

//libsignon-qt
#include <SignOn/AuthSession>
#include <SignOn/SessionData>


AuthSessionInterfacePrivate::AuthSessionInterfacePrivate(SignOn::AuthSession *authSession, AuthSessionInterface *parent)
    : QObject(parent), q(parent), session(authSession), status(AuthSessionInterface::NotStarted), error(AuthSessionInterface::NoError)
{
    connect(session, SIGNAL(error(SignOn::Error)), this, SLOT(handleError(SignOn::Error)));
    connect(session, SIGNAL(mechanismsAvailable(QStringList)), this, SLOT(handleMechanismsAvailable(QStringList)));
    connect(session, SIGNAL(response(SignOn::SessionData)), this, SLOT(handleResponse(SignOn::SessionData)));
    connect(session, SIGNAL(stateChanged(SignOn::AuthSession::AuthSessionState, QString)), this, SLOT(handleStateChanged(SignOn::AuthSession::AuthSessionState, QString)));

    methodName = session->name();
    session->queryAvailableMechanisms();
}

AuthSessionInterfacePrivate::~AuthSessionInterfacePrivate()
{
}

void AuthSessionInterfacePrivate::handleError(const SignOn::Error &err)
{
    error = static_cast<AuthSessionInterface::ErrorType>(err.type());
    errorMessage = err.message();
    if (status != AuthSessionInterface::Invalid) {
        status = AuthSessionInterface::Error;
        emit q->statusChanged();
    }
    emit q->errorChanged();
    emit q->errorMessageChanged();
}

void AuthSessionInterfacePrivate::handleMechanismsAvailable(const QStringList &mechs)
{
    mechanisms = mechs;
    emit q->mechanismsChanged();
}

void AuthSessionInterfacePrivate::handleResponse(const SignOn::SessionData &sd)
{
    // XXX TODO: should we pass back a SessionDataInterface ptr instead?  Ownership concerns?
    QVariantMap dataMap;
    QStringList keys = sd.propertyNames();
    foreach (const QString &key, keys)
        dataMap.insert(key, sd.getProperty(key));
    emit q->response(dataMap);
}

void AuthSessionInterfacePrivate::handleStateChanged(SignOn::AuthSession::AuthSessionState newState, const QString &message)
{
    if (status == AuthSessionInterface::Invalid)
        return; // currently unused, but in the future we may want to detect deletion of Identity or something.

    status = static_cast<AuthSessionInterface::Status>(newState);
    statusMessage = message;
    emit q->statusChanged();
    emit q->statusMessageChanged();
}

//------------------------------

/*!
    \qmltype AuthSession
    \instantiates AuthSessionInterface
    \inqmlmodule org.nemomobile.signon 1
    \brief Used to authenticate with a service provider

    This type is intended for use by client applications who
    wish to sign on to a service.  They will have retrieved
    information about the credentials from a ServiceAccount
    and instantiated a ServiceAccountIdentity, from which the
    AuthSession may be created.
*/

AuthSessionInterface::AuthSessionInterface(SignOn::AuthSession *session, QObject *parent)
    : QObject(parent), d(new AuthSessionInterfacePrivate(session, this))
{
}

AuthSessionInterface::~AuthSessionInterface()
{
}

SignOn::AuthSession *AuthSessionInterface::authSession() const
{
    // Helper method for ServiceAccountIdentityInterface.  Do not use elsewhere.
    return d->session;
}

/*!
    \qmlmethod void AuthSession::process(SessionData *sessionData, const QString &mechanism)
    Process an authentication request contained in the \a sessionData using the given \a mechanism.
    The \a sessionData should include credentials including userName, secret, etc.
*/
void AuthSessionInterface::process(SessionDataInterface *sessionData, const QString &mechanism)
{
    if (!sessionData)
        return;
    d->session->process(sessionData->sessionData(), mechanism);
}

/*!
    \qmlmethod void AuthSession::process(const QVariantMap &sessionData, const QString &mechanism)
    Process an authentication request contained in the \a sessionData using the given \a mechanism.
    The \a sessionData should include credentials including userName, secret, etc.
*/
void AuthSessionInterface::process(const QVariantMap &sessionData, const QString &mechanism)
{
    d->session->process(SignOn::SessionData(sessionData), mechanism);
}

// TODO: remove request() as it is mostly unused
void AuthSessionInterface::request(SessionDataInterface *sessionData, const QString &mechanism)
{
    if (!sessionData)
        return;
    d->session->request(sessionData->sessionData(), mechanism);
}

// TODO: remove challenge() as it is mostly unused
void AuthSessionInterface::challenge(SessionDataInterface *sessionData, const QString &mechanism)
{
    if (!sessionData)
        return;
    d->session->challenge(sessionData->sessionData(), mechanism);
}

// TODO: remove cancelChallenge() as challenge() is mostly unused
void AuthSessionInterface::cancelChallenge()
{
    d->session->cancel();
}

/*!
    \qmlmethod void AuthSession::refreshMechanisms()

    Triggers a refresh of the authentication mechanisms
    which may be used during signon.  This function may
    may return immediately (if the accounts database is
    unlocked) or asynchronously.

    Once the mechanisms list has been refreshed, the
    mechanismsChanged() signal will be emitted, even if
    the list of mechanisms contains the same values.
*/
void AuthSessionInterface::refreshMechanisms()
{
    d->session->queryAvailableMechanisms();
}

/*!
    \qmlproperty string AuthSession::methodName
    Contains the name of the method which will be used to
    authenticate with the service via this AuthSession
*/
QString AuthSessionInterface::methodName() const
{
    return d->methodName;
}

/*!
    \qmlproperty QStringList AuthSession::mechanisms
    Contains the mechanisms which may be used to authenticate
    with the service via this AuthSession.
*/ 
QStringList AuthSessionInterface::mechanisms() const
{
    return d->mechanisms;
}

/*!
    \qmlproperty AuthSessionInterface::Status AuthSession::status
    Contains the current status of this AuthSession
*/
AuthSessionInterface::Status AuthSessionInterface::status() const
{
    return d->status;
}

/*!
    \qmlproperty string AuthSession::statusMessage
    Contains the message associated with the current status of this AuthSession
*/
QString AuthSessionInterface::statusMessage() const
{
    return d->statusMessage;
}

/*!
    \qmlproperty AuthSessionInterface::ErrorType AuthSession::error
    Contains the most recent error which occurred during creation or
    use of the AuthSession.  Note that this property will not be
    updated to NoError if subsequent signon processes succeed.
*/
AuthSessionInterface::ErrorType AuthSessionInterface::error() const
{
    return d->error;
}

/*!
    \qmlproperty QString AuthSession::errorMessage
    Contains the message associated with the most recent error which
    occurred during creation or use of the AuthSession.  Note that
    this property will not be updated if subsequent signon processes succeed.
*/
QString AuthSessionInterface::errorMessage() const
{
    return d->errorMessage;
}

