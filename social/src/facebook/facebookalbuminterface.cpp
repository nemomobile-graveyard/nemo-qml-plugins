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

#include "facebookalbuminterface.h"
#include "facebookalbuminterface_p.h"

#include "facebookontology_p.h"
#include "facebookobjectreferenceinterface.h"
#include "identifiablecontentiteminterface_p.h"
#include "contentiteminterface_p.h"

#include <QtDebug>

FacebookAlbumInterfacePrivate::FacebookAlbumInterfacePrivate(FacebookAlbumInterface *parent, IdentifiableContentItemInterfacePrivate *parentData)
    : QObject(parent)
    , q(parent)
    , dd(parentData)
    , from(new FacebookObjectReferenceInterface(this))
    , action(FacebookInterfacePrivate::NoAction)
    , liked(false)
{
}

FacebookAlbumInterfacePrivate::~FacebookAlbumInterfacePrivate()
{
    dd->deleteReply();
}

/*! \internal */
void FacebookAlbumInterfacePrivate::finishedHandler()
{
    if (!dd->reply()) {
        // if an error occurred, it might have been deleted by the error handler.
        qWarning() << Q_FUNC_INFO << "network request finished but no reply";
        return;
    }

    QByteArray replyData = dd->reply()->readAll();
    dd->deleteReply();
    bool ok = false;
    QVariantMap responseData = ContentItemInterface::parseReplyData(replyData, &ok);
    if (!ok)
        responseData.insert("response", replyData);

    switch (action) {
        case FacebookInterfacePrivate::LikeAction:        // flow down.
        case FacebookInterfacePrivate::DeleteLikeAction:  // flow down.
        case FacebookInterfacePrivate::DeletePhotoAction: // flow down.
        case FacebookInterfacePrivate::DeleteCommentAction: {
            if (replyData == QString(QLatin1String("true"))) {
                dd->status = SocialNetworkInterface::Idle;
                if (action == FacebookInterfacePrivate::LikeAction) {
                    liked = true;
                    emit q->likedChanged();
                } else if (action == FacebookInterfacePrivate::DeleteLikeAction) {
                    liked = false;
                    emit q->likedChanged();
                }
                emit q->statusChanged();
                emit q->responseReceived(responseData);
            } else {
                dd->error = SocialNetworkInterface::RequestError;
                dd->errorMessage = QLatin1String("Album: request failed");
                dd->status = SocialNetworkInterface::Error;
                emit q->statusChanged();
                emit q->errorChanged();
                emit q->errorMessageChanged();
                emit q->responseReceived(responseData);
            }
        }
        break;

        case FacebookInterfacePrivate::UploadPhotoAction: // flow down.
        case FacebookInterfacePrivate::UploadCommentAction: {
            if (!ok || responseData.value("id").toString().isEmpty()) {
                // failed.
                dd->error = SocialNetworkInterface::RequestError;
                dd->errorMessage = action == FacebookInterfacePrivate::UploadCommentAction
                    ? QLatin1String("Album: add comment request failed")
                    : QLatin1String("Album: add photo request failed");
                dd->status = SocialNetworkInterface::Error;
                emit q->statusChanged();
                emit q->errorChanged();
                emit q->errorMessageChanged();
                emit q->responseReceived(responseData);
            } else {
                // succeeded.
                dd->status = SocialNetworkInterface::Idle;
                emit q->statusChanged();
                emit q->responseReceived(responseData);
            }
        }
        break;

        default: {
            dd->error = SocialNetworkInterface::OtherError;
            dd->errorMessage = QLatin1String("Request finished but no action currently in progress");
            dd->status = SocialNetworkInterface::Error;
            emit q->statusChanged();
            emit q->errorChanged();
            emit q->errorMessageChanged();
            emit q->responseReceived(responseData);
        }
        break;
    }
}

//----------------------------------

/*!
    \qmltype FacebookAlbum
    \instantiates FacebookAlbumInterface
    \inqmlmodule org.nemomobile.social 1
    \brief A FacebookAlbum represents an Album object from the Facebook OpenGraph API

    Every album has a unique identifier, and thus an album may be
    set as the \c node (or central content item) in the Facebook
    adapter.  The content items related to an album include various
    photos or videos, likes and comments.

    An example of usage of a FacebookAlbum as the node in a Facebook
    model is as follows:

    \qml
    import QtQuick 1.1
    import org.nemomobile.social 1.0

    Item {
        id: root
        width: 400
        height: 800

        Flickable {
            anchors.top: parent.verticalCenter
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right

            ListView {
                model: fb
                anchors.fill: parent
                delegate: Label { text: "id: " + contentItemIdentifier } // Photo ids
            }
        }

        Facebook {
            id: fb
            accessToken: "your access token"    // you must supply a valid access token
            nodeIdentifier: "10150146071791729" // some valid Facebook album id.
            filters: [ ContentItemTypeFilter { type: Facebook.Photo } ]
        }

        Component.onCompleted: {
            fb.populate()
        }
    }
    \endqml

    A FacebookAlbum may also be used "directly" by clients, in order to
    upload photos or comments, or like the album.  An example of direct
    usage of the FacebookAlbum type is as follows:

    \qml
    import QtQuick 1.1
    import org.nemomobile.social 1.0

    Item {
        id: root
        width: 400
        height: 800

        Facebook {
            id: fb
            accessToken: "your access token"    // you must supply a valid access token
        }

        FacebookAlbum {
            id: fba
            socialNetwork: fb
            identifier: "10150146071791729"     // some valid Facebook Album fbid

            onStatusChanged: {
                if (status == SocialNetwork.Idle) {
                    // could comment on the album
                    fba.uploadComment("I really like this album!")
                    // could like the album
                    fba.like()
                    // could unlike the album
                    fba.unlike()
                    // could upload another photo to the album
                    fba.uploadPhoto(fileName, "A photo I took recently")
                }
            }
        }
    }
    \endqml
*/

FacebookAlbumInterface::FacebookAlbumInterface(QObject *parent)
    : IdentifiableContentItemInterface(parent), f(new FacebookAlbumInterfacePrivate(this, dd))
{
}

FacebookAlbumInterface::~FacebookAlbumInterface()
{
}

/*! \reimp */
int FacebookAlbumInterface::type() const
{
    return FacebookInterface::Album;
}

/*! \reimp */
bool FacebookAlbumInterface::remove()
{
    return IdentifiableContentItemInterface::remove();
}

/*! \reimp */
bool FacebookAlbumInterface::reload(const QStringList &whichFields)
{
    return IdentifiableContentItemInterface::reload(whichFields);
}

/*! \reimp */
void FacebookAlbumInterface::emitPropertyChangeSignals(const QVariantMap &oldData, const QVariantMap &newData)
{
    QVariantMap fromMap = newData.value(FACEBOOK_ONTOLOGY_ALBUM_FROM).toMap();
    QString fromIdStr = fromMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString fromNameStr = fromMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();
    QString anStr = newData.value(FACEBOOK_ONTOLOGY_ALBUM_NAME).toString();
    QString descStr = newData.value(FACEBOOK_ONTOLOGY_ALBUM_DESCRIPTION).toString();
    QString linkStr = newData.value(FACEBOOK_ONTOLOGY_ALBUM_LINK).toString();
    QString cpStr = newData.value(FACEBOOK_ONTOLOGY_ALBUM_COVERPHOTO).toString();
    QString prvStr = newData.value(FACEBOOK_ONTOLOGY_ALBUM_PRIVACY).toString();
    QString countStr = newData.value(FACEBOOK_ONTOLOGY_ALBUM_COUNT).toString();
    QString typeStr = newData.value(FACEBOOK_ONTOLOGY_ALBUM_TYPE).toString();
    QString ctStr = newData.value(FACEBOOK_ONTOLOGY_ALBUM_CREATEDTIME).toString();
    QString utStr = newData.value(FACEBOOK_ONTOLOGY_ALBUM_UPDATEDTIME).toString();
    QString cuStr = newData.value(FACEBOOK_ONTOLOGY_ALBUM_CANUPLOAD).toString();

    QVariantMap oldFromMap = oldData.value(FACEBOOK_ONTOLOGY_ALBUM_FROM).toMap();
    QString oldFromIdStr = oldFromMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString oldFromNameStr = oldFromMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();
    QString oldAnStr = oldData.value(FACEBOOK_ONTOLOGY_ALBUM_NAME).toString();
    QString oldDescStr = oldData.value(FACEBOOK_ONTOLOGY_ALBUM_DESCRIPTION).toString();
    QString oldLinkStr = oldData.value(FACEBOOK_ONTOLOGY_ALBUM_LINK).toString();
    QString oldCpStr = oldData.value(FACEBOOK_ONTOLOGY_ALBUM_COVERPHOTO).toString();
    QString oldPrvStr = oldData.value(FACEBOOK_ONTOLOGY_ALBUM_PRIVACY).toString();
    QString oldCountStr = oldData.value(FACEBOOK_ONTOLOGY_ALBUM_COUNT).toString();
    QString oldTypeStr = oldData.value(FACEBOOK_ONTOLOGY_ALBUM_TYPE).toString();
    QString oldCtStr = oldData.value(FACEBOOK_ONTOLOGY_ALBUM_CREATEDTIME).toString();
    QString oldUtStr = oldData.value(FACEBOOK_ONTOLOGY_ALBUM_UPDATEDTIME).toString();
    QString oldCuStr = oldData.value(FACEBOOK_ONTOLOGY_ALBUM_CANUPLOAD).toString();

    if (anStr != oldAnStr)
        emit nameChanged();
    if (descStr != oldDescStr)
        emit descriptionChanged();
    if (linkStr != oldLinkStr)
        emit linkChanged();
    if (cpStr != oldCpStr)
        emit coverPhotoChanged();
    if (prvStr != oldPrvStr)
        emit privacyChanged();
    if (countStr != oldCountStr)
        emit countChanged();
    if (typeStr != oldTypeStr)
        emit albumTypeChanged();
    if (ctStr != oldCtStr)
        emit createdTimeChanged();
    if (utStr != oldUtStr)
        emit updatedTimeChanged();
    if (cuStr != oldCuStr)
        emit canUploadChanged();

    // update the from object if required
    if (fromIdStr != oldFromIdStr || fromNameStr != oldFromNameStr) {
        QVariantMap newFromData;
        newFromData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTTYPE, FacebookInterface::User);
        newFromData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER, fromIdStr);
        newFromData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME, fromNameStr);
        qobject_cast<FacebookInterface*>(socialNetwork())->setFacebookContentItemData(f->from, newFromData);
        emit fromChanged();
    }

    // call the super class implementation
    IdentifiableContentItemInterface::emitPropertyChangeSignals(oldData, newData);
}

/*!
    \qmlmethod bool FacebookAlbum::like()
    Initiates a "like" operation on the album.

    If the network request was started successfully, the function
    will return true and the status of the album will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.
*/
bool FacebookAlbumInterface::like()
{
    bool requestMade = request(IdentifiableContentItemInterface::Post,
                               identifier(), QLatin1String("likes"));

    if (!requestMade)
        return false;

    f->action = FacebookInterfacePrivate::LikeAction;
    connect(f->dd->reply(), SIGNAL(finished()), f, SLOT(finishedHandler()));
    connect(f->dd->reply(), SIGNAL(error(QNetworkReply::NetworkError)), f->dd, SLOT(defaultErrorHandler(QNetworkReply::NetworkError)));
    connect(f->dd->reply(), SIGNAL(sslErrors(QList<QSslError>)), f->dd, SLOT(defaultSslErrorsHandler(QList<QSslError>)));
    return true;
}

/*!
    \qmlmethod bool FacebookAlbum::unlike()
    Initiates a "delete like" operation on the album.

    If the network request was started successfully, the function
    will return true and the status of the album will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.
*/
bool FacebookAlbumInterface::unlike()
{
    bool requestMade = request(IdentifiableContentItemInterface::Delete,
                               identifier(), QLatin1String("likes"));

    if (!requestMade)
        return false;

    f->action = FacebookInterfacePrivate::DeleteLikeAction;
    connect(f->dd->reply(), SIGNAL(finished()), f, SLOT(finishedHandler()));
    connect(f->dd->reply(), SIGNAL(error(QNetworkReply::NetworkError)), f->dd, SLOT(defaultErrorHandler(QNetworkReply::NetworkError)));
    connect(f->dd->reply(), SIGNAL(sslErrors(QList<QSslError>)), f->dd, SLOT(defaultSslErrorsHandler(QList<QSslError>)));
    return true;
}

/*!
    \qmlmethod bool FacebookAlbum::uploadComment(const QString &message)
    Initiates a "post comment" operation on the album.  The comment
    will contain the specified \a message.

    If the network request was started successfully, the function
    will return true and the status of the album will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.

    Once the network request completes, the \c responseReceived()
    signal will be emitted.  The \c data parameter of the signal
    will contain the \c id of the newly uploaded comment.
*/
bool FacebookAlbumInterface::uploadComment(const QString &message)
{
    QVariantMap postData;
    postData.insert("message", message);

    bool requestMade = request(IdentifiableContentItemInterface::Post,
                               identifier(), QLatin1String("comments"),
                               QStringList(), postData);

    if (!requestMade)
        return false;

    f->action = FacebookInterfacePrivate::UploadCommentAction;
    connect(f->dd->reply(), SIGNAL(finished()), f, SLOT(finishedHandler()));
    connect(f->dd->reply(), SIGNAL(error(QNetworkReply::NetworkError)), f->dd, SLOT(defaultErrorHandler(QNetworkReply::NetworkError)));
    connect(f->dd->reply(), SIGNAL(sslErrors(QList<QSslError>)), f->dd, SLOT(defaultSslErrorsHandler(QList<QSslError>)));
    return true;
}

/*!
    \qmlmethod bool FacebookAlbum::removeComment(const QString &identifier)
    Initiates a "delete comment" operation on the comment specified by
    the given \a identifier.

    If the network request was started successfully, the function
    will return true and the status of the album will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.
*/
bool FacebookAlbumInterface::removeComment(const QString &commentIdentifier)
{
    bool requestMade = request(IdentifiableContentItemInterface::Delete, commentIdentifier);

    if (!requestMade)
        return false;

    f->action = FacebookInterfacePrivate::DeleteCommentAction;
    connect(f->dd->reply(), SIGNAL(finished()), f, SLOT(finishedHandler()));
    connect(f->dd->reply(), SIGNAL(error(QNetworkReply::NetworkError)), f->dd, SLOT(defaultErrorHandler(QNetworkReply::NetworkError)));
    connect(f->dd->reply(), SIGNAL(sslErrors(QList<QSslError>)), f->dd, SLOT(defaultSslErrorsHandler(QList<QSslError>)));
    return true;
}

/*!
    \qmlmethod bool FacebookAlbum::uploadPhoto(const QUrl &source, const QString &message)
    Initiates a "post photo" operation on the album.  The photo will
    be loaded from the local filesystem and uploaded to Facebook with
    its caption set to the given \a message.

    If the network request was started successfully, the function
    will return true and the status of the album will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.

    Once the network request completes, the \c responseReceived()
    signal will be emitted.  The \c data parameter of the signal
    will contain the \c id of the newly uploaded photo.
*/
bool FacebookAlbumInterface::uploadPhoto(const QUrl &source, const QString &message)
{
    // XXX TODO: privacy parameter?

    QVariantMap extraData; // image upload is handled specially by the facebook adapter.
    extraData.insert("isImageUpload", true);

    QVariantMap postData;
    postData.insert("source", source);
    if (!message.isEmpty())
        postData.insert("message", message);

    bool requestMade = request(IdentifiableContentItemInterface::Post,
                               identifier(), QLatin1String("photos"),
                               QStringList(), postData, extraData);

    if (!requestMade)
        return false;

    f->action = FacebookInterfacePrivate::UploadPhotoAction;
    connect(f->dd->reply(), SIGNAL(finished()), f, SLOT(finishedHandler()));
    connect(f->dd->reply(), SIGNAL(error(QNetworkReply::NetworkError)), f->dd, SLOT(defaultErrorHandler(QNetworkReply::NetworkError)));
    connect(f->dd->reply(), SIGNAL(sslErrors(QList<QSslError>)), f->dd, SLOT(defaultSslErrorsHandler(QList<QSslError>)));
    return true;
}

/*!
    \qmlmethod bool FacebookAlbum::removePhoto(const QString &identifier)
    Initiates a "delete photo" operation on the photo specified by
    the given \a identifier.

    If the network request was started successfully, the function
    will return true and the status of the album will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.
*/
bool FacebookAlbumInterface::removePhoto(const QString &photoIdentifier)
{
    bool requestMade = request(IdentifiableContentItemInterface::Delete, photoIdentifier);

    if (!requestMade)
        return false;

    f->action = FacebookInterfacePrivate::DeletePhotoAction;
    connect(f->dd->reply(), SIGNAL(finished()), f, SLOT(finishedHandler()));
    connect(f->dd->reply(), SIGNAL(error(QNetworkReply::NetworkError)), f->dd, SLOT(defaultErrorHandler(QNetworkReply::NetworkError)));
    connect(f->dd->reply(), SIGNAL(sslErrors(QList<QSslError>)), f->dd, SLOT(defaultSslErrorsHandler(QList<QSslError>)));
    return true;
}

/*!
    \qmlproperty FacebookObjectReference *FacebookAlbum::from
    Holds a reference to the user or profile which created the album.
*/
FacebookObjectReferenceInterface *FacebookAlbumInterface::from() const
{
    return f->from;
}

/*!
    \qmlproperty QString FacebookAlbum::name
    Holds the name of the album
*/
QString FacebookAlbumInterface::name() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_ALBUM_NAME).toString();
}

/*!
    \qmlproperty QString FacebookAlbum::description
    Holds the description of the album
*/
QString FacebookAlbumInterface::description() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_ALBUM_DESCRIPTION).toString();
}

/*!
    \qmlproperty QUrl FacebookAlbum::link
    Holds a link to the album
*/
QUrl FacebookAlbumInterface::link() const
{
    return QUrl(d->data().value(FACEBOOK_ONTOLOGY_ALBUM_LINK).toString());
}

/*!
    \qmlproperty QString FacebookAlbum::coverPhoto
    Holds a link to the cover photo of an album
*/
QString FacebookAlbumInterface::coverPhoto() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_ALBUM_COVERPHOTO).toString();
}

/*!
    \qmlproperty QString FacebookAlbum::privacy
    Holds the privacy setting of the album
*/
QString FacebookAlbumInterface::privacy() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_ALBUM_PRIVACY).toString();
}

/*!
    \qmlproperty int FacebookAlbum::count
    Holds the count of the number of photos in the album
*/
int FacebookAlbumInterface::count() const
{
    QString countStr = d->data().value(FACEBOOK_ONTOLOGY_ALBUM_COUNT).toString();
    bool ok = false;
    int retn = countStr.toInt(&ok);
    if (ok)
        return retn;
    return -1;
}

/*!
    \qmlproperty FacebookAlbum::AlbumType FacebookAlbum::albumType
    Holds the type of the album.  Valid values are:
    \list
    \li FacebookAlbum.Album
    \li FacebookAlbum.Normal
    \li FacebookAlbum.Wall
    \li FacebookAlbum.Profile
    \li FacebookAlbum.Mobile
    \endlist
*/
FacebookAlbumInterface::AlbumType FacebookAlbumInterface::albumType() const
{
    QString atStr = d->data().value(FACEBOOK_ONTOLOGY_ALBUM_TYPE).toString().toLower();
    if (atStr == QLatin1String("normal"))
        return FacebookAlbumInterface::Normal;
    else if (atStr == QLatin1String("wall"))
        return FacebookAlbumInterface::Wall;
    else if (atStr == QLatin1String("profile"))
        return FacebookAlbumInterface::Profile;
    else if (atStr == QLatin1String("mobile"))
        return FacebookAlbumInterface::Mobile;
    return FacebookAlbumInterface::Album;
}

/*!
    \qmlproperty QString FacebookAlbum::createdTime
    Holds the creation time of the album in an ISO8601-formatted string
*/
QString FacebookAlbumInterface::createdTime() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_ALBUM_CREATEDTIME).toString();
}

/*!
    \qmlproperty QString FacebookAlbum::updatedTime
    Holds the last-update time of the album in an ISO8601-formatted string
*/
QString FacebookAlbumInterface::updatedTime() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_ALBUM_UPDATEDTIME).toString();
}

/*!
    \qmlproperty bool FacebookAlbum::canUpload
    Whether the current user can upload photos to the album
*/
bool FacebookAlbumInterface::canUpload() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_ALBUM_CANUPLOAD).toString() == QLatin1String("true");
}

/*!
    \qmlproperty bool FacebookAlbum::liked
    Whether the album has been liked by the current user
*/
bool FacebookAlbumInterface::liked() const
{
    return f->liked;
}

