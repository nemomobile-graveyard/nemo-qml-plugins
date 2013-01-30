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

#ifndef SESSIONDATAINTERFACE_H
#define SESSIONDATAINTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QVariantMap>

//libsignon-qt
#include <SignOn/SessionData>

class SessionDataInterfacePrivate;
class AuthSessionInterface;

/*
 * SessionDataInterface
 *
 * Lightweight QML interface for libsignon-qt's SessionData.
 * The session data is a simple dictionary of key/value pairs.
 * It is used as the container for authentication session
 * parameters and results.
 *
 * It is intended for use by 3rd party application developers.
 */
class SessionDataInterface : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QStringList accessControlTokens READ accessControlTokens CONSTANT)
    Q_PROPERTY(QVariantMap properties READ properties WRITE setProperties NOTIFY propertiesChanged)

public:
    SessionDataInterface(SignOn::SessionData data = SignOn::SessionData(), QObject *parent = 0);
    ~SessionDataInterface();

    // property accessors
    QStringList accessControlTokens();
    QVariantMap properties() const;
    void setProperties(const QVariantMap &data);

Q_SIGNALS:
    void propertiesChanged();

public:
    static QVariantMap sanitizeVariantMap(const QVariantMap &input);

private:
    SignOn::SessionData sessionData() const;
    friend class AuthSessionInterface;

private:
    SessionDataInterfacePrivate *d;
    friend class SessionDataInterfacePrivate;
};

#endif
