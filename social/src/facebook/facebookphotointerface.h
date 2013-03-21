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

#ifndef FACEBOOKPHOTOINTERFACE_H
#define FACEBOOKPHOTOINTERFACE_H

#include "identifiablecontentiteminterface.h"

#include "facebooktaginterface.h"

#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include <QtCore/QStringList>
#include <QtCore/QString>
#include <QtCore/QUrl>

#include <QtDeclarative/QDeclarativeListProperty>

class FacebookObjectReferenceInterface;

/*
 * NOTE: if you construct one of these in C++ directly,
 * you MUST call classBegin() and componentCompleted()
 * directly after construction.
 */

class FacebookPhotoInterfacePrivate;
class FacebookPhotoInterface : public IdentifiableContentItemInterface
{
    Q_OBJECT

    Q_PROPERTY(QString albumIdentifier READ albumIdentifier NOTIFY albumIdentifierChanged)
    Q_PROPERTY(FacebookObjectReferenceInterface *from READ from NOTIFY fromChanged)
    Q_PROPERTY(QDeclarativeListProperty<FacebookTagInterface> tags READ tags NOTIFY tagsChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged) // "name" from FB API, kind of a caption
    Q_PROPERTY(QVariantMap nameTags READ nameTags NOTIFY nameTagsChanged) // /me vomits
    Q_PROPERTY(QUrl icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(QUrl picture READ picture NOTIFY pictureChanged)
    Q_PROPERTY(QUrl source READ source NOTIFY sourceChanged)
    Q_PROPERTY(int height READ height NOTIFY heightChanged)
    Q_PROPERTY(int width READ width NOTIFY widthChanged)
    Q_PROPERTY(QVariantMap images READ images NOTIFY imagesChanged)
    Q_PROPERTY(QUrl link READ link NOTIFY linkChanged)
    Q_PROPERTY(QVariantMap place READ place NOTIFY placeChanged)
    Q_PROPERTY(QString createdTime READ createdTime NOTIFY createdTimeChanged)
    Q_PROPERTY(QString updatedTime READ updatedTime NOTIFY updatedTimeChanged)
    Q_PROPERTY(int position READ position NOTIFY positionChanged) // album position

    Q_PROPERTY(bool liked READ liked NOTIFY likedChanged) // requires: requesting all likes connections, iterating through, finding "current user id" / or not.

public:
    FacebookPhotoInterface(QObject *parent = 0);

    // overrides.
    int type() const;
    Q_INVOKABLE bool remove();
    Q_INVOKABLE bool reload(const QStringList &whichFields = QStringList());
    void emitPropertyChangeSignals(const QVariantMap &oldData, const QVariantMap &newData);

    // invokable API
    Q_INVOKABLE bool like();
    Q_INVOKABLE bool unlike();
    Q_INVOKABLE bool tagUser(const QString &userId, qreal xOffset = -1, qreal yOffset = -1);
    Q_INVOKABLE bool untagUser(const QString &userId);
    Q_INVOKABLE bool tagText(const QString &text, qreal xOffset = -1, qreal yOffset = -1);
    Q_INVOKABLE bool untagText(const QString &text);
    Q_INVOKABLE bool uploadComment(const QString &message);
    Q_INVOKABLE bool removeComment(const QString &commentIdentifier);

public:
    // property accessors.
    QString albumIdentifier() const;
    FacebookObjectReferenceInterface *from() const;
    QDeclarativeListProperty<FacebookTagInterface> tags();
    QString name() const;
    QVariantMap nameTags() const;
    QUrl icon() const;
    QUrl picture() const;
    QUrl source() const;
    int height() const;
    int width() const;
    QVariantMap images() const;
    QUrl link() const;
    QVariantMap place() const;
    QString createdTime() const;
    QString updatedTime() const;
    int position() const;
    bool liked() const;

Q_SIGNALS:
    void albumIdentifierChanged();
    void fromChanged();
    void tagsChanged();
    void nameChanged();
    void nameTagsChanged();
    void iconChanged();
    void pictureChanged();
    void sourceChanged();
    void heightChanged();
    void widthChanged();
    void imagesChanged();
    void linkChanged();
    void placeChanged();
    void createdTimeChanged();
    void updatedTimeChanged();
    void positionChanged();
    void likedChanged();
private:
    Q_DECLARE_PRIVATE(FacebookPhotoInterface)
};

#endif // FACEBOOKPHOTOINTERFACE_H
