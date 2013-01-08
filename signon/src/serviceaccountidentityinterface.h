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

#ifndef SERVICEACCOUNTIDENTITYINTERFACE_H
#define SERVICEACCOUNTIDENTITYINTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include <QtCore/QStringList>
#include <QtCore/QString>

//libsignon-qt
#include <SignOn/Identity>
#include <SignOn/AuthSession>

class ServiceAccountIdentityInterfacePrivate;
class SessionDataInterface;

/*
 * ServiceAccountIdentityInterface
 *
 * Provides an interface to an Identity in QML for use with a service account.
 * It provides API to create an authentication session with this identity.
 * It also provides API to request updated credentials from the user via UI plugin.
 *
 * This is intended to be used by 3rd party application developers.
 * It may be retrieved from a ServiceAccount.
 */

class ServiceAccountIdentityInterface : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int identifier READ identifier WRITE setIdentifier NOTIFY identifierChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(ErrorType error READ error NOTIFY errorChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

    Q_PROPERTY(QStringList methods READ methods NOTIFY methodsChanged)

    Q_ENUMS(Status)
    Q_ENUMS(ErrorType)

public:
    enum Status {
        NotStarted          = SignOn::AuthSession::SessionNotStarted,
        HostResolving       = SignOn::AuthSession::HostResolving,
        ServerConnecting    = SignOn::AuthSession::ServerConnecting,
        DataSending         = SignOn::AuthSession::DataSending,
        ReplyWaiting        = SignOn::AuthSession::ReplyWaiting,
        UserPending         = SignOn::AuthSession::UserPending,
        UiRefreshing        = SignOn::AuthSession::UiRefreshing,
        ProcessPending      = SignOn::AuthSession::ProcessPending,
        SessionStarted      = SignOn::AuthSession::SessionStarted,
        ProcessCanceling    = SignOn::AuthSession::ProcessCanceling,
        ProcessDone         = SignOn::AuthSession::ProcessDone,
        CustomState         = SignOn::AuthSession::CustomState,
        Initialized         = SignOn::AuthSession::MaxState+1,
        Initializing,        
        Error,
        Invalid
    };

    enum ErrorType {
        UnknownError                = SignOn::AuthSession::UnknownError,
        InternalServerError         = SignOn::AuthSession::InternalServerError,
        InternalCommunicationError  = SignOn::AuthSession::InternalCommunicationError,
        PermissionDeniedError       = SignOn::AuthSession::PermissionDeniedError,
        AuthSessionError            = SignOn::AuthSession::AuthSessionErr,
        MechanismNotAvailableError  = SignOn::AuthSession::MechanismNotAvailableError,
        MissingDataError            = SignOn::AuthSession::MissingDataError,
        InvalidCredentialsError     = SignOn::AuthSession::InvalidCredentialsError,
        WrongStateError             = SignOn::AuthSession::WrongStateError,
        OperationNotSupportedError  = SignOn::AuthSession::OperationNotSupportedError,
        NoConnectionError           = SignOn::AuthSession::NoConnectionError,
        NetworkError                = SignOn::AuthSession::NetworkError,
        SslError                    = SignOn::AuthSession::SslError,
        RuntimeError                = SignOn::AuthSession::RuntimeError,
        CanceledError               = SignOn::AuthSession::CanceledError,
        TimedOutError               = SignOn::AuthSession::TimedOutError,
        UserInteractionError        = SignOn::AuthSession::UserInteractionError,
        UserError                   = SignOn::Error::UserErr,
        NoError                     = UserError+1
    };

public:
    ServiceAccountIdentityInterface(SignOn::Identity *ident = 0, QObject *parent = 0);
    ~ServiceAccountIdentityInterface();

    // invokable API
    Q_INVOKABLE QStringList methodMechanisms(const QString &methodName) const;
    Q_INVOKABLE void verifySecret(const QString &secret);

    // UI-related: these cause a dialog to be spawned
    Q_INVOKABLE void requestCredentialsUpdate(const QString &message);
    Q_INVOKABLE void verifyUser(const QString &message);
    Q_INVOKABLE void verifyUser(const QVariantMap &params);

    // session-related:
    Q_INVOKABLE bool signIn(const QString &methodName, const QString &mechanism, const QVariantMap &sessionData);
    Q_INVOKABLE void process(const QVariantMap &sessionData);
    Q_INVOKABLE void signOut();

    // property accessors and mutators
    void setIdentifier(int identityId);
    int identifier() const;
    ErrorType error() const;
    QString errorMessage() const;
    Status status() const;
    QString statusMessage() const;
    QStringList methods() const;

Q_SIGNALS:
    void responseReceived(const QVariantMap &data);
    void signedOut();
    void secretVerified(bool valid);
    void userVerified(bool valid);

    void identifierChanged();
    void errorChanged();
    void errorMessageChanged();
    void statusChanged();
    void statusMessageChanged();
    void methodsChanged();

private:
    ServiceAccountIdentityInterfacePrivate *d;
    friend class ServiceAccountIdentityInterfacePrivate;
};

#endif
