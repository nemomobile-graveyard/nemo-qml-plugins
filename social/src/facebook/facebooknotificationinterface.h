/*
 * Copyright (C) 2013 Jolla Ltd. <chris.adams@jollamobile.com>
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

#ifndef FACEBOOKNOTIFICATIONINTERFACE_H
#define FACEBOOKNOTIFICATIONINTERFACE_H

#include "identifiablecontentiteminterface.h"

#include <QtCore/QVariantMap>
#include <QtCore/QString>
#include <QtCore/QUrl>

class FacebookObjectReferenceInterface;

/*
 * NOTE: if you construct one of these in C++ directly,
 * you MUST call classBegin() and componentCompleted()
 * directly after construction.
 */

class FacebookNotificationInterfacePrivate;
class FacebookNotificationInterface : public IdentifiableContentItemInterface
{
    Q_OBJECT
    Q_PROPERTY(FacebookObjectReferenceInterface *from READ from NOTIFY fromChanged)
    Q_PROPERTY(FacebookObjectReferenceInterface *to READ to NOTIFY toChanged)
    Q_PROPERTY(FacebookObjectReferenceInterface *application READ application NOTIFY applicationChanged)
    Q_PROPERTY(QString createdTime READ createdTime NOTIFY createdTimeChanged)
    Q_PROPERTY(QString updatedTime READ updatedTime NOTIFY updatedTimeChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QUrl link READ link NOTIFY linkChanged)
    Q_PROPERTY(int unread READ unread NOTIFY unreadChanged)
    /* XXX TODO: "object" property -- probably another object reference? Undocumented. */

public:
    FacebookNotificationInterface(QObject *parent = 0);
    ~FacebookNotificationInterface();

    // overrides
    int type() const;
    Q_INVOKABLE bool remove();
    Q_INVOKABLE bool reload(const QStringList &whichFields = QStringList());
    void emitPropertyChangeSignals(const QVariantMap &oldData, const QVariantMap &newData);

    // properties
    FacebookObjectReferenceInterface *from() const;
    FacebookObjectReferenceInterface *to() const;
    FacebookObjectReferenceInterface *application() const;
    QString createdTime() const;
    QString updatedTime() const;
    QString title() const;
    QUrl link() const;
    int unread() const;

Q_SIGNALS:
    void fromChanged();
    void toChanged();
    void applicationChanged();
    void createdTimeChanged();
    void updatedTimeChanged();
    void titleChanged();
    void linkChanged();
    void unreadChanged();

protected:
    FacebookNotificationInterfacePrivate *f;
    friend class FacebookNotificationInterfacePrivate;
    Q_DISABLE_COPY(FacebookNotificationInterface)
};

#endif // FACEBOOKNOTIFICATIONINTERFACE_H
