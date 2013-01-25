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

#ifndef FACEBOOKINTERFACE_H
#define FACEBOOKINTERFACE_H

#include "socialnetworkinterface.h"

#include <QtCore/QStringList>
#include <QtCore/QString>

class QNetworkReply;

class FacebookAlbumInterface;
class FacebookCommentInterface;
class FacebookLikeInterface;
class FacebookObjectReferenceInterface;
class FacebookPhotoInterface;
class FacebookPictureInterface;
class FacebookTagInterface;
class FacebookUserInterface;
class IdentifiableContentItemInterface;

/*
 * NOTE: if you construct one of these in C++ directly,
 * you MUST call classBegin() and componentCompleted()
 * directly after construction.
 */

class CacheEntry;
class FacebookInterfacePrivate;
class FacebookInterface : public SocialNetworkInterface
{
    Q_OBJECT

    Q_PROPERTY(QString accessToken READ accessToken WRITE setAccessToken NOTIFY accessTokenChanged)

    Q_ENUMS(ContentItemType)

public:
    enum ContentItemType {
        NotInitialized = 0,
        Unknown = 1,
        ObjectReference,
        Like,
        Tag,
        Picture,
        Location,
        Comment,
        User,
        Album,
        Photo,
        Event
    };

public:
    FacebookInterface(QObject *parent = 0);
    ~FacebookInterface();

    // properties
    QString accessToken() const;
    void setAccessToken(const QString &token);

Q_SIGNALS:
    void accessTokenChanged();

    // SocialNetworkInterface
public:
    void componentComplete();
    Q_INVOKABLE void populate();
protected:
    QNetworkReply *getRequest(const QString &objectIdentifier, const QString &extraPath, const QStringList &whichFields, const QVariantMap &extraData);
    QNetworkReply *postRequest(const QString &objectIdentifier, const QString &extraPath, const QVariantMap &data, const QVariantMap &extraData);
    QNetworkReply *deleteRequest(const QString &objectIdentifier, const QString &extraPath, const QVariantMap &extraData);
    void updateInternalData(QList<CacheEntry*> data);
    void populateDataForNode(IdentifiableContentItemInterface *currentNode);
    void populateDataForNode(const QString &unseenNodeIdentifier);
    ContentItemInterface *contentItemFromData(QObject *parent, const QVariantMap &data) const;

    // private API for all Facebook adapters to use
private:
    QString currentUserIdentifier() const;
    FacebookObjectReferenceInterface *objectReference(QObject *parent, int type, QString identifier, QString name);
    QVariantMap facebookContentItemData(ContentItemInterface *contentItem);
    void setFacebookContentItemData(ContentItemInterface *contentItem, const QVariantMap &data);
    friend class FacebookAlbumInterface;
    friend class FacebookCommentInterface;
    friend class FacebookLikeInterface;
    friend class FacebookObjectReferenceInterface;
    friend class FacebookPhotoInterface;
    friend class FacebookPictureInterface;
    friend class FacebookTagInterface;
    friend class FacebookUserInterface;

    // impl. detail
private:
    void retrieveRelatedContent(IdentifiableContentItemInterface *whichNode);
    void continuePopulateDataForUnseenNode(const QVariantMap &nodeData);
    void continuePopulateDataForSeenNode(const QVariantMap &nodeData);

    // private data.
private:
    FacebookInterfacePrivate *f;
    friend class FacebookInterfacePrivate;
    Q_DISABLE_COPY(FacebookInterface)
};

#endif // FACEBOOKINTERFACE_H
