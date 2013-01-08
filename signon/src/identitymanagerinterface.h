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

#ifndef IDENTITYMANAGERINTERFACE_H
#define IDENTITYMANAGERINTERFACE_H

#include <QtCore/QObject>

class IdentityManagerInterfacePrivate;
class IdentityInterface;

/*
 * IdentityManagerInterface
 *
 * Allow creation and removal of identities.
 * Also, allows adding and removing references to identities.
 *
 * Note that this class doesn't "mirror" anything directly in
 * the underlying libsignon-qt API; instead, it merely exposes
 * the functionality of libsignon-qt's Identity class which
 * should not be accessible to everyone.
 *
 * This class is intended for use by restricted plugins,
 * not 3rd party application developers.
 */
class IdentityManagerInterface : public QObject
{
    Q_OBJECT

public:
    IdentityManagerInterface(QObject *parent = 0);
    ~IdentityManagerInterface();

    // invokable API
    Q_INVOKABLE IdentityInterface *createIdentity();
    Q_INVOKABLE void removeIdentity(IdentityInterface *identity);
    Q_INVOKABLE IdentityInterface *identity(const QString &identityIdentifier);
    Q_INVOKABLE void addIdentityReference(IdentityInterface *identity, const QString &reference);
    Q_INVOKABLE void removeIdentityReference(IdentityInterface *identity, const QString &reference);

private:
    IdentityManagerInterfacePrivate *d;
    friend class IdentityManagerInterfacePrivate;
};

#endif
