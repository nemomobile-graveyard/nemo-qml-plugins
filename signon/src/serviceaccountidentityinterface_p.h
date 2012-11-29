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

#ifndef SERVICEACCOUNTIDENTITYINTERFACE_P_H
#define SERVICEACCOUNTIDENTITYINTERFACE_P_H

#include "serviceaccountidentityinterface.h"

#include <QtCore/QObject>

//libsignon-qt
#include <SignOn/Identity>

class ServiceAccountIdentityInterfacePrivate : public QObject
{
    Q_OBJECT

public:
    ServiceAccountIdentityInterfacePrivate(SignOn::Identity *ident, ServiceAccountIdentityInterface *parent);
    ~ServiceAccountIdentityInterfacePrivate();

    void setIdentity(SignOn::Identity *ident, bool emitSignals, bool takeOwnership);
    void setStatus(ServiceAccountIdentityInterface::Status newStatus);

    ServiceAccountIdentityInterface *q;
    SignOn::Identity *identity;
    bool ownIdentity; // only true if constructed with 0 identity initially!
    bool finishedInitialization; // false until we update our available methods.

    ServiceAccountIdentityInterface::Status status;
    ServiceAccountIdentityInterface::ErrorType error;
    QString errorMessage;

    QStringList availableMethods;

public Q_SLOTS:
    void handleError(SignOn::Error err);
    void handleRemoved();
    void handleMethodsAvailable(const QStringList &methods);
};

#endif
