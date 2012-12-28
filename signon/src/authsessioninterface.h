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

#ifndef AUTHSESSIONINTERFACE_H
#define AUTHSESSIONINTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include <QtCore/QStringList>
#include <QtCore/QString>

//libsignon-qt
#include <SignOn/AuthSession>

class AuthSessionInterfacePrivate;
class SessionDataInterface;
class ServiceAccountIdentityInterface;

/*
 * AuthSessionInterface
 *
 * Provides a QML interface to libsignon-qt's AuthSession.
 * Allows 3rd party application developers to authenticate.
 * The AuthSessionInterface should be acquired from a
 * ServiceAccountIdentityInterface instance.
 */
class AuthSessionInterface : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString methodName READ methodName CONSTANT)
    Q_PROPERTY(QStringList mechanisms READ mechanisms NOTIFY mechanismsChanged)

    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(ErrorType error READ error NOTIFY errorChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

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
        MaxState            = SignOn::AuthSession::MaxState,
        Error               = MaxState+1,
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
    AuthSessionInterface(SignOn::AuthSession *session, QObject *parent);
    ~AuthSessionInterface();

    // invokable api
    Q_INVOKABLE void process(const QVariantMap &sessionData, const QString &mechanism);
    Q_INVOKABLE void process(SessionDataInterface *sessionData, const QString &mechanism);
    Q_INVOKABLE void request(SessionDataInterface *sessionData, const QString &mechanism);
    Q_INVOKABLE void challenge(SessionDataInterface *sessionData, const QString &mechanism);
    Q_INVOKABLE void cancelChallenge(); 
    Q_INVOKABLE void refreshMechanisms();

    // property accessors
    QString methodName() const;
    QStringList mechanisms() const;
    Status status() const;
    QString statusMessage() const;
    ErrorType error() const;
    QString errorMessage() const;

Q_SIGNALS:
    void mechanismsChanged();
    void statusChanged();
    void statusMessageChanged();
    void errorChanged();
    void errorMessageChanged();
    void response(const QVariantMap &sessionData);

private:
    SignOn::AuthSession *authSession() const;
    friend class ServiceAccountIdentityInterface;

private:
    AuthSessionInterfacePrivate *d;
    friend class AuthSessionInterfacePrivate;
};

#endif
