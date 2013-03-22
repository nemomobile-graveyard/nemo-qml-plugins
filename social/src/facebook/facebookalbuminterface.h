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

#ifndef FACEBOOKALBUMINTERFACE_H
#define FACEBOOKALBUMINTERFACE_H

#include "identifiablecontentiteminterface.h"

#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include <QtCore/QStringList>
#include <QtCore/QString>
#include <QtCore/QUrl>

class FacebookObjectReferenceInterface;

/*
 * NOTE: if you construct one of these in C++ directly,
 * you MUST call classBegin() and componentCompleted()
 * directly after construction.
 */

class FacebookAlbumInterfacePrivate;
class FacebookAlbumInterface : public IdentifiableContentItemInterface
{
    Q_OBJECT

    Q_PROPERTY(FacebookObjectReferenceInterface *from READ from NOTIFY fromChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
    Q_PROPERTY(QUrl link READ link NOTIFY linkChanged)
    Q_PROPERTY(QString coverPhoto READ coverPhoto NOTIFY coverPhotoChanged)
    Q_PROPERTY(QString privacy READ privacy NOTIFY privacyChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(AlbumType albumType READ albumType NOTIFY albumTypeChanged)
    Q_PROPERTY(QString createdTime READ createdTime NOTIFY createdTimeChanged)
    Q_PROPERTY(QString updatedTime READ updatedTime NOTIFY updatedTimeChanged)
    Q_PROPERTY(bool canUpload READ canUpload NOTIFY canUploadChanged)

    Q_PROPERTY(bool liked READ liked NOTIFY likedChanged) // requires: requesting all likes connections, iterating through, finding "current user id" / or not...

    Q_ENUMS(AlbumType)

public:
    enum AlbumType {
        Album,
        Normal,
        Wall,
        Profile,
        Mobile
    };

public:
    explicit FacebookAlbumInterface(QObject *parent = 0);

    // overrides.
    int type() const;
    Q_INVOKABLE bool remove();
    Q_INVOKABLE bool reload(const QStringList &whichFields = QStringList());

    // invokable API
    Q_INVOKABLE bool like();
    Q_INVOKABLE bool unlike();
    Q_INVOKABLE bool uploadComment(const QString &message);
    Q_INVOKABLE bool removeComment(const QString &commentIdentifier);
    Q_INVOKABLE bool uploadPhoto(const QUrl &source, const QString &message = QString());
    Q_INVOKABLE bool removePhoto(const QString &photoIdentifier);

public:
    // property accessors.
    FacebookObjectReferenceInterface *from() const;
    QString name() const;
    QString description() const;
    QUrl link() const;
    QString coverPhoto() const;
    QString privacy() const;
    int count() const;
    AlbumType albumType() const;
    QString createdTime() const;
    QString updatedTime() const;
    bool canUpload() const;
    bool liked() const;

Q_SIGNALS:
    void fromChanged();
    void nameChanged();
    void descriptionChanged();
    void linkChanged();
    void coverPhotoChanged();
    void privacyChanged();
    void countChanged();
    void albumTypeChanged();
    void createdTimeChanged();
    void updatedTimeChanged();
    void canUploadChanged();
    void likedChanged();
private:
    Q_DECLARE_PRIVATE(FacebookAlbumInterface)
};

#endif // FACEBOOKALBUMINTERFACE_H
