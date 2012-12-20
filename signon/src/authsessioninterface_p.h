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

#ifndef AUTHSESSIONINTERFACE_P_H
#define AUTHSESSIONINTERFACE_P_H

#include "authsessioninterface.h"

#include <QtCore/QObject>

//libsignon-qt
#include <SignOn/AuthSession>

class AuthSessionInterfacePrivate : public QObject
{
    Q_OBJECT

public:
    AuthSessionInterfacePrivate(SignOn::AuthSession *authSession, AuthSessionInterface *parent);
    ~AuthSessionInterfacePrivate();

    AuthSessionInterface *q;
    SignOn::AuthSession *session;

    AuthSessionInterface::Status status;
    AuthSessionInterface::ErrorType error;
    QString statusMessage;
    QString errorMessage;

    QString methodName;
    QStringList mechanisms;

public Q_SLOTS:
    void handleError(const SignOn::Error &err);
    void handleMechanismsAvailable(const QStringList &mechs);
    void handleResponse(const SignOn::SessionData &sd);
    void handleStateChanged(SignOn::AuthSession::AuthSessionState newState, const QString &message);
};

#endif
