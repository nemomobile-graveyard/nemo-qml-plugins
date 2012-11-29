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

#include <QtGlobal>
#include <QtDeclarative>
#include <QDeclarativeEngine>
#include <QDeclarativeExtensionPlugin>

#include "authsessioninterface.h"
#include "identityinterface.h"
#include "identitymanagerinterface.h"
#include "serviceaccountidentityinterface.h"
#include "sessiondatainterface.h"

class Q_DECL_EXPORT NemoSignonPlugin : public QDeclarativeExtensionPlugin
{
public:
    virtual ~NemoSignonPlugin() { }

    void initializeEngine(QDeclarativeEngine *engine, const char *uri)
    {
        Q_ASSERT(uri == QLatin1String("org.nemomobile.signon"));
        Q_UNUSED(engine)
        Q_UNUSED(uri)
    }

    void registerTypes(const char *uri)
    {
        Q_ASSERT(uri == QLatin1String("org.nemomobile.signon"));

        QString m2 = QLatin1String("Retrieve from ServiceAccountIdentity");

        // Types which should be exposed to "normal" applications:
        qmlRegisterType<ServiceAccountIdentityInterface>(uri, 1, 0, "ServiceAccountIdentity");
        qmlRegisterType<SessionDataInterface>(uri, 1, 0, "SessionData");
        qmlRegisterUncreatableType<AuthSessionInterface>(uri, 1, 0, "AuthSession", m2);

        // Types which should be exposed to "settings" application:
        qmlRegisterType<IdentityManagerInterface>(uri, 1, 0, "IdentityManager");
        qmlRegisterType<IdentityInterface>(uri, 1, 0, "Identity");
    }
};

Q_EXPORT_PLUGIN2(nemosignon, NemoSignonPlugin);

