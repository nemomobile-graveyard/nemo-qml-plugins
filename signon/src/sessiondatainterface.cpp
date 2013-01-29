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

#include "sessiondatainterface.h"

//libsignon-qt
#include <SignOn/SessionData>

class SessionDataInterfacePrivate
{
public:
    SignOn::SessionData sessionData;
    QVariantMap propertyValues;
};

//--------------------------------------------

/*!
    \qmltype SessionData
    \instantiates SessionDataInterface
    \inqmlmodule org.nemomobile.signon 1
    \brief Session data associated with an authentication session

    This type is intended for use by client applications who
    wish to authenticate with services.  Each authentication session
    signon process requires a SessionData to be specified, which
    includes information like userName, secret, caption, etc.
*/

SessionDataInterface::SessionDataInterface(SignOn::SessionData data, QObject *parent)
    : QObject(parent), d(new SessionDataInterfacePrivate)
{
    d->sessionData = data;
    QStringList pnames = d->sessionData.propertyNames();
    foreach (const QString &key, pnames)
        d->propertyValues.insert(key, d->sessionData.getProperty(key));
}

SessionDataInterface::~SessionDataInterface()
{
    delete d;
}

/*!
    \qmlproperty QStringList SessionData::accessControlTokens
    Contains the access control tokens which apply to the current application using the session data
*/

QStringList SessionDataInterface::accessControlTokens()
{
    return d->sessionData.getAccessControlTokens();
}

/*!
    \qmlproperty QVariantMap SessionData::properties
    Contains the key/value pairs which make up the session data.
*/

QVariantMap SessionDataInterface::properties() const
{
    return d->propertyValues;
}

void SessionDataInterface::setProperties(const QVariantMap &data)
{
    d->propertyValues = data;
    d->sessionData = SignOn::SessionData(data);
    emit propertiesChanged();
}

QVariantMap SessionDataInterface::sanitizeVariantMap(const QVariantMap &input)
{
    // Helper function for Identity/ServiceAccountIdentity.  Don't use this elsewhere.
    QVariantMap retn;

    QStringList allkeys = input.keys();
    foreach (const QString &key, allkeys) {
        QVariant curr = input.value(key);
        if (curr.type() == QVariant::List) {
            retn.insert(key, curr.toStringList());
        } else {
            retn.insert(key, curr);
        }
    }

    return retn;
}

SignOn::SessionData SessionDataInterface::sessionData() const
{
    // Helper function for Identity/ServiceAccountIdentity.  Don't use this elsewhere.
    return d->sessionData;
}

