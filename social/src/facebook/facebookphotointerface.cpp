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

#include "facebookphotointerface.h"
#include "facebookphotointerface_p.h"

#include "facebookontology_p.h"
#include "facebookobjectreferenceinterface.h"
#include "identifiablecontentiteminterface_p.h"
#include "contentiteminterface_p.h"

#include <QtDebug>

FacebookPhotoInterfacePrivate::FacebookPhotoInterfacePrivate(FacebookPhotoInterface *q)
    : IdentifiableContentItemInterfacePrivate(q)
    // , from(new FacebookObjectReferenceInterface(this)) // Unsafe
    , pendingTagToRemoveIndex(-1)
    , action(FacebookInterfacePrivate::NoAction)
    , liked(false)
{
}

FacebookPhotoInterfacePrivate::~FacebookPhotoInterfacePrivate()
{
    deleteReply();
}

void FacebookPhotoInterfacePrivate::finishedHandler()
{
    Q_Q(FacebookPhotoInterface);
    if (!reply()) {
        // if an error occurred, it might have been deleted by the error handler.
        qWarning() << Q_FUNC_INFO << "network request finished but no reply";
        return;
    }

    QByteArray replyData = reply()->readAll();
    deleteReply();
    bool ok = false;
    QVariantMap responseData = ContentItemInterface::parseReplyData(replyData, &ok);
    if (!ok)
        responseData.insert("response", replyData);

    switch (action) {
        case FacebookInterfacePrivate::LikeAction:       // flow
        case FacebookInterfacePrivate::DeleteLikeAction: // flow
        case FacebookInterfacePrivate::TagAction:        // flow
        case FacebookInterfacePrivate::DeleteTagAction:  // flow
        case FacebookInterfacePrivate::DeleteCommentAction: {
            if (replyData == QString(QLatin1String("true"))) {
                status = SocialNetworkInterface::Idle;
                if (action == FacebookInterfacePrivate::LikeAction) {
                    liked = true;
                    emit q->likedChanged();
                } else if (action == FacebookInterfacePrivate::DeleteLikeAction) {
                    liked = false;
                    emit q->likedChanged();
                } else if (action == FacebookInterfacePrivate::DeleteTagAction) {
                    if (pendingTagToRemoveIndex != -1) {
                        FacebookTagInterface *doomedTag = tags.takeAt(pendingTagToRemoveIndex);
                        pendingTagToRemoveIndex = -1;
                        doomedTag->deleteLater();
                        emit q->tagsChanged();
                    }
                }
                emit q->statusChanged();
                emit q->responseReceived(responseData);
            } else {
                if (pendingTagToRemoveIndex != -1)
                    pendingTagToRemoveIndex = -1;
                error = SocialNetworkInterface::RequestError;
                errorMessage = QLatin1String("Photo: request failed");
                status = SocialNetworkInterface::Error;
                emit q->statusChanged();
                emit q->errorChanged();
                emit q->errorMessageChanged();
                emit q->responseReceived(responseData);
            }
        }
        break;

        case FacebookInterfacePrivate::UploadCommentAction: {
            if (!ok || responseData.value("id").toString().isEmpty()) {
                // failed.
                error = SocialNetworkInterface::RequestError;
                errorMessage = QLatin1String("Photo: add comment request failed");
                status = SocialNetworkInterface::Error;
                emit q->statusChanged();
                emit q->errorChanged();
                emit q->errorMessageChanged();
                emit q->responseReceived(responseData);
            } else {
                // succeeded.
                status = SocialNetworkInterface::Idle;
                emit q->statusChanged();
                emit q->responseReceived(responseData);
            }
        }
        break;

        default: {
            error = SocialNetworkInterface::OtherError;
            errorMessage = QLatin1String("Request finished but no action currently in progress");
            status = SocialNetworkInterface::Error;
            emit q->statusChanged();
            emit q->errorChanged();
            emit q->errorMessageChanged();
            emit q->responseReceived(responseData);
        }
        break;
    }
}


/*! \internal */
void FacebookPhotoInterfacePrivate::tags_append(QDeclarativeListProperty<FacebookTagInterface> *list, FacebookTagInterface *tag)
{
    FacebookPhotoInterface *fci = qobject_cast<FacebookPhotoInterface *>(list->object);
    if (fci) {
        tag->setParent(fci);
        fci->d_func()->tags.append(tag);
    }
}

/*! \internal */
FacebookTagInterface *FacebookPhotoInterfacePrivate::tags_at(QDeclarativeListProperty<FacebookTagInterface> *list, int index)
{
    FacebookPhotoInterface *fci = qobject_cast<FacebookPhotoInterface *>(list->object);
    if (fci && index < fci->d_func()->tags.count() && index >= 0)
        return fci->d_func()->tags.at(index);
    return 0;
}

/*! \internal */
void FacebookPhotoInterfacePrivate::tags_clear(QDeclarativeListProperty<FacebookTagInterface> *list)
{
    FacebookPhotoInterface *fci = qobject_cast<FacebookPhotoInterface *>(list->object);
    if (fci) {
        foreach (FacebookTagInterface *tag, fci->d_func()->tags)
            tag->deleteLater();
        fci->d_func()->tags.clear();
    }
}

/*! \internal */
int FacebookPhotoInterfacePrivate::tags_count(QDeclarativeListProperty<FacebookTagInterface> *list)
{
    FacebookPhotoInterface *fci = qobject_cast<FacebookPhotoInterface *>(list->object);
    if (fci)
        return fci->d_func()->tags.count();
    return 0;
}

//---------------------------------------

/*!
    \qmltype FacebookPhoto
    \instantiates FacebookPhotoInterface
    \inqmlmodule org.nemomobile.social 1
    \brief A FacebookPhoto represents a Photo object from the Facebook OpenGraph API

    Every FacebookPhoto has a unique identifier, and thus a photo may be
    set as the \c node (or central content item) in the Facebook
    adapter.  The content items related to a photo include various
    likes and comments.

    An example of usage of a FacebookPhoto as the node in a Facebook
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
                delegate: Label { text: "id: " + contentItemIdentifier } // Comment ids
            }
        }

        Facebook {
            id: fb
            accessToken: "your access token"    // you must supply a valid access token
            nodeIdentifier: "10150146071966729" // some valid Facebook photo id.
            filters: [ ContentItemTypeFilter { type: Facebook.Comment } ]
        }

        Component.onCompleted: {
            fb.populate()
        }
    }
    \endqml

    A FacebookPhoto may also be used "directly" by clients, in order to
    upload comments, or like the photo.  An example of direct
    usage of the FacebookPhoto type is as follows:

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

        FacebookPhoto {
            id: fbph
            socialNetwork: fb
            identifier: "10150146071966729"     // some valid Facebook Photo fbid

            onStatusChanged: {
                if (status == SocialNetwork.Idle) {
                    // could comment on the photo
                    fbph.uploadComment("I really like this photo!")
                    // could like the photo
                    fbph.like()
                    // could unlike the photo
                    fbph.unlike()
                }
            }
        }

        Image {
            anchors.fill: parent
            source: fbph.source
        }
    }
    \endqml
*/

FacebookPhotoInterface::FacebookPhotoInterface(QObject *parent)
    : IdentifiableContentItemInterface(*(new FacebookPhotoInterfacePrivate(this)), parent)
{
    Q_D(FacebookPhotoInterface);
    d->from = new FacebookObjectReferenceInterface(this);
}

FacebookPhotoInterface::~FacebookPhotoInterface()
{
}

/*! \reimp */
int FacebookPhotoInterface::type() const
{
    return FacebookInterface::Photo;
}

/*! \reimp */
bool FacebookPhotoInterface::remove()
{
    return IdentifiableContentItemInterface::remove();
}

/*! \reimp */
bool FacebookPhotoInterface::reload(const QStringList &whichFields)
{
    return IdentifiableContentItemInterface::reload(whichFields);
}

/*! \reimp */
void FacebookPhotoInterface::emitPropertyChangeSignals(const QVariantMap &oldData, const QVariantMap &newData)
{
    Q_D(FacebookPhotoInterface);
    QString aidStr = newData.value(FACEBOOK_ONTOLOGY_PHOTO_ALBUMIDENTIFIER).toString();
    QVariantList tagsList = newData.value(FACEBOOK_ONTOLOGY_PHOTO_TAGS).toList();
    QVariantMap fromMap = newData.value(FACEBOOK_ONTOLOGY_PHOTO_FROM).toMap();
    QString nameStr = newData.value(FACEBOOK_ONTOLOGY_PHOTO_NAME).toString();
    QVariantMap ntMap = newData.value(FACEBOOK_ONTOLOGY_PHOTO_NAMETAGS).toMap();
    QString iconStr = newData.value(FACEBOOK_ONTOLOGY_PHOTO_ICON).toString();
    QString picStr = newData.value(FACEBOOK_ONTOLOGY_PHOTO_PICTURE).toString();
    QString srcStr = newData.value(FACEBOOK_ONTOLOGY_PHOTO_SOURCE).toString();
    QString heightInt = newData.value(FACEBOOK_ONTOLOGY_PHOTO_HEIGHT).toString();
    QString widthInt = newData.value(FACEBOOK_ONTOLOGY_PHOTO_WIDTH).toString();
    QVariantMap imgsMap = newData.value(FACEBOOK_ONTOLOGY_PHOTO_IMAGES).toMap();
    QString linkStr = newData.value(FACEBOOK_ONTOLOGY_PHOTO_LINK).toString();
    QVariantMap placeMap = newData.value(FACEBOOK_ONTOLOGY_PHOTO_PLACE).toMap();     // XXX TODO: Location/Place object reference
    QString ctStr = newData.value(FACEBOOK_ONTOLOGY_PHOTO_CREATEDTIME).toString();
    QString utStr = newData.value(FACEBOOK_ONTOLOGY_PHOTO_UPDATEDTIME).toString();
    QString posInt = newData.value(FACEBOOK_ONTOLOGY_PHOTO_POSITION).toString();
    QString likedBool = newData.value(FACEBOOK_ONTOLOGY_PHOTO_LIKED).toString();

    QString oldAidStr = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_ALBUMIDENTIFIER).toString();
    QVariantList oldTagsList = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_TAGS).toList();
    QVariantMap oldFromMap = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_FROM).toMap();
    QString oldNameStr = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_NAME).toString();
    QVariantMap oldNtMap = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_NAMETAGS).toMap();
    QString oldIconStr = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_ICON).toString();
    QString oldPicStr = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_PICTURE).toString();
    QString oldSrcStr = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_SOURCE).toString();
    QString oldHeightInt = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_HEIGHT).toString();
    QString oldWidthInt = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_WIDTH).toString();
    QVariantMap oldImgsMap = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_IMAGES).toMap();
    QString oldLinkStr = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_LINK).toString();
    QVariantMap oldPlaceMap = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_PLACE).toMap();     // XXX TODO: Location/Place object reference
    QString oldCtStr = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_CREATEDTIME).toString();
    QString oldUtStr = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_UPDATEDTIME).toString();
    QString oldPosInt = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_POSITION).toString();
    QString oldLikedBool = oldData.value(FACEBOOK_ONTOLOGY_PHOTO_LIKED).toString();

    if (aidStr != oldAidStr)
        emit albumIdentifierChanged();
    if (nameStr != oldNameStr)
        emit nameChanged();
    if (ntMap != oldNtMap)
        emit nameTagsChanged();
    if (iconStr != oldIconStr)
        emit iconChanged();
    if (picStr != oldPicStr)
        emit pictureChanged();
    if (srcStr != oldSrcStr)
        emit sourceChanged();
    if (heightInt != oldHeightInt)
        emit heightChanged();
    if (widthInt != oldWidthInt)
        emit widthChanged();
    if (imgsMap != oldImgsMap)
        emit imagesChanged();
    if (linkStr != oldLinkStr)
        emit linkChanged();
    if (placeMap != oldPlaceMap)
        emit placeChanged();
    if (ctStr != oldCtStr)
        emit createdTimeChanged();
    if (utStr != oldUtStr)
        emit updatedTimeChanged();
    if (posInt != oldPosInt)
        emit positionChanged();
    if (likedBool != oldLikedBool) {
        d->liked = likedBool == QLatin1String("true");
        emit likedChanged();
    }

    // update tags list
    if (tagsList != oldTagsList) {
        // clear the old tags
        foreach (FacebookTagInterface *doomedTag, d->tags)
            doomedTag->deleteLater();
        d->tags.clear();

        // update with the new tag data
        for (int i = 0; i < tagsList.size(); ++i) {
            QVariantMap currTagMap = tagsList.at(i).toMap();
            currTagMap.insert(FACEBOOK_ONTOLOGY_TAG_TARGETIDENTIFIER, identifier());
            FacebookTagInterface *currTag = new FacebookTagInterface(this);
            qobject_cast<FacebookInterface*>(socialNetwork())->setFacebookContentItemData(currTag, currTagMap);
            d->tags.append(currTag);
        }

        // emit change signal
        emit tagsChanged();
    }

    // update from reference
    if (fromMap != oldFromMap) {
        QVariantMap newFromData;
        newFromData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTTYPE, FacebookInterface::User); // could also be a Profile ...
        newFromData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER, fromMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER));
        newFromData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME, fromMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME));
        qobject_cast<FacebookInterface*>(socialNetwork())->setFacebookContentItemData(d->from, newFromData);
        emit fromChanged();
    }

    // call the super class implementation
    QVariantMap oldDataWithId = oldData; oldDataWithId.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID, oldData.value(FACEBOOK_ONTOLOGY_PHOTO_ID));
    QVariantMap newDataWithId = newData; newDataWithId.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID, newData.value(FACEBOOK_ONTOLOGY_PHOTO_ID));
    IdentifiableContentItemInterface::emitPropertyChangeSignals(oldDataWithId, newDataWithId);
}

/*!
    \qmlmethod bool FacebookPhoto::like()
    Initiates a "like" operation on the photo.

    If the network request was started successfully, the function
    will return true and the status of the photo will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.
*/
bool FacebookPhotoInterface::like()
{
    Q_D(FacebookPhotoInterface);
    bool requestMade = request(IdentifiableContentItemInterface::Post,
                               identifier(), QLatin1String("likes"));

    if (!requestMade)
        return false;

    d->action = FacebookInterfacePrivate::LikeAction;
    d->connectFinishedAndErrors();
    return true;
}

/*!
    \qmlmethod bool FacebookPhoto::unlike()
    Initiates a "delete like" operation on the photo.

    If the network request was started successfully, the function
    will return true and the status of the photo will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.
*/
bool FacebookPhotoInterface::unlike()
{
    Q_D(FacebookPhotoInterface);
    bool requestMade = request(IdentifiableContentItemInterface::Delete,
                               identifier(), QLatin1String("likes"));

    if (!requestMade)
        return false;

    d->action = FacebookInterfacePrivate::DeleteLikeAction;
    d->connectFinishedAndErrors();
    return true;
}

/*!
    \qmlmethod bool FacebookPhoto::tagUser(const QString &userId, qreal xOffset, qreal yOffset)
    Initiates a "tag user" operation on the photo.  The user specified
    by the given \a userId will be tagged into the photo at the position
    specified by the given \a xOffset and \a yOffset.

    If the network request was started successfully, the function
    will return true and the status of the photo will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.

    Once the network request completes, the \c responseReceived()
    signal will be emitted.
*/
bool FacebookPhotoInterface::tagUser(const QString &userId, qreal xOffset, qreal yOffset)
{
    Q_D(FacebookPhotoInterface);
    QVariantMap postData;
    postData.insert("id", userId);
    if (xOffset != -1)
        postData.insert("x", QString::number(xOffset));
    if (yOffset != -1)
        postData.insert("y", QString::number(yOffset));

    bool requestMade = request(IdentifiableContentItemInterface::Post,
                               identifier(), QLatin1String("tags"),
                               QStringList(), postData);

    if (!requestMade)
        return false;

    d->action = FacebookInterfacePrivate::TagAction;
    d->connectFinishedAndErrors();
    return true;
}

/*!
    \qmlmethod bool FacebookPhoto::untagUser(const QString &userId)
    Initiates a "delete tag" operation on the tag which tags the
    user specified by the given \a userId into the photo.

    If the network request was started successfully, the function
    will return true and the status of the photo will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.
*/
bool FacebookPhotoInterface::untagUser(const QString &userId)
{
    Q_D(FacebookPhotoInterface);
    QVariantMap extraData;
    extraData.insert("to", userId);

    // try to find which tag will be removed if it succeeds
    int tempPendingTagToRemoveIndex = -1;
    for (int i = 0; i < d->tags.count(); ++i) {
        QString tagUid = d->tags.at(i)->userIdentifier();
        if (!tagUid.isEmpty() && tagUid == userId) {
            tempPendingTagToRemoveIndex = i;
            break;
        }
    }

    // possible that it's ok to not exist, since we might be out of sync with reality.
    if (tempPendingTagToRemoveIndex == -1)
        qWarning() << Q_FUNC_INFO << "Unknown tag specified for removal";

    bool requestMade = request(IdentifiableContentItemInterface::Delete,
                               identifier(), QLatin1String("tags"),
                               QStringList(), QVariantMap(), extraData);

    if (!requestMade)
        return false;

    d->action = FacebookInterfacePrivate::DeleteTagAction;
    d->pendingTagToRemoveIndex = tempPendingTagToRemoveIndex;
    d->connectFinishedAndErrors();
    return true;
}

/*!
    \qmlmethod bool FacebookPhoto::tagText(const QString &text, qreal xOffset, qreal yOffset)
    Initiates a "tag text" operation on the photo.  The position
    specified by the given \a xOffset and \a yOffset will be tagged
    with the specified \a text.

    If the network request was started successfully, the function
    will return true and the status of the photo will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.

    Once the network request completes, the \c responseReceived()
    signal will be emitted.
*/
bool FacebookPhotoInterface::tagText(const QString &text, qreal xOffset, qreal yOffset)
{
    Q_D(FacebookPhotoInterface);
    QVariantMap postData;
    postData.insert("tag_text", text);
    if (xOffset != -1)
        postData.insert("x", QString::number(xOffset));
    if (yOffset != -1)
        postData.insert("y", QString::number(yOffset));

    bool requestMade = request(IdentifiableContentItemInterface::Post,
                               identifier(), QLatin1String("tags"),
                               QStringList(), postData);

    if (!requestMade)
        return false;

    d->action = FacebookInterfacePrivate::TagAction;
    d->connectFinishedAndErrors();
    return true;
}

/*!
    \qmlmethod bool FacebookPhoto::untagText(const QString &text)
    Initiates a "delete tag" operation on the tag specified by
    the given text.

    If the network request was started successfully, the function
    will return true and the status of the photo will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.
*/
bool FacebookPhotoInterface::untagText(const QString &text)
{
    Q_D(FacebookPhotoInterface);
    QVariantMap extraData;
    extraData.insert("tag_text", text);

    // try to find which tag will be removed if it succeeds
    int tempPendingTagToRemoveIndex = -1;
    for (int i = 0; i < d->tags.count(); ++i) {
        QString tagText = d->tags.at(i)->text();
        if (!tagText.isEmpty() && tagText == text) {
            tempPendingTagToRemoveIndex = i;
            break;
        }
    }

    // possible that it's ok to not exist, since we might be out of sync with reality.
    if (tempPendingTagToRemoveIndex == -1)
        qWarning() << Q_FUNC_INFO << "Unknown tag specified for removal";

    bool requestMade = request(IdentifiableContentItemInterface::Delete,
                               identifier(), QLatin1String("tags"),
                               QStringList(), QVariantMap(), extraData);

    if (!requestMade)
        return false;

    d->action = FacebookInterfacePrivate::DeleteTagAction;
    d->pendingTagToRemoveIndex = tempPendingTagToRemoveIndex;
    d->connectFinishedAndErrors();
    return true;
}

/*!
    \qmlmethod bool FacebookPhoto::uploadComment(const QString &message)
    Initiates a "post comment" operation on the photo.  The comment
    will contain the specified \a message.

    If the network request was started successfully, the function
    will return true and the status of the photo will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.

    Once the network request completes, the \c responseReceived()
    signal will be emitted.  The \c data parameter of the signal
    will contain the \c id of the newly uploaded comment.
*/
bool FacebookPhotoInterface::uploadComment(const QString &message)
{
    Q_D(FacebookPhotoInterface);
    QVariantMap postData;
    postData.insert("message", message);

    bool requestMade = request(IdentifiableContentItemInterface::Post,
                               identifier(), QLatin1String("comments"),
                               QStringList(), postData);

    if (!requestMade)
        return false;

    d->action = FacebookInterfacePrivate::UploadCommentAction;
    d->connectFinishedAndErrors();
    return true;
}

/*!
    \qmlmethod bool FacebookPhoto::removeComment(const QString &identifier)
    Initiates a "delete comment" operation on the comment specified by
    the given \a identifier.

    If the network request was started successfully, the function
    will return true and the status of the photo will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.
*/
bool FacebookPhotoInterface::removeComment(const QString &commentIdentifier)
{
    Q_D(FacebookPhotoInterface);
    bool requestMade = request(IdentifiableContentItemInterface::Delete, commentIdentifier);

    if (!requestMade)
        return false;

    d->action = FacebookInterfacePrivate::DeleteCommentAction;
    d->connectFinishedAndErrors();
    return true;
}


/*!
    \qmlproperty QString FacebookPhoto::albumIdentifier
    Holds the identifier of the album to which this photo belongs.
    Note: this property is currently not implemented.
    XXX TODO: implement this.
*/
QString FacebookPhotoInterface::albumIdentifier() const
{
    Q_D(const FacebookPhotoInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_PHOTO_ALBUMIDENTIFIER).toString();
}

/*!
    \qmlproperty FacebookObjectReference *FacebookPhoto::from
    Holds a reference to the user or profile which uploaded this photo
*/
FacebookObjectReferenceInterface *FacebookPhotoInterface::from() const
{
    Q_D(const FacebookPhotoInterface);
    return d->from;
}

/*!
    \qmlproperty QDeclarativeListProperty<FacebookTag> FacebookPhoto::tags
    Holds the tags which have been uploaded for this photo
*/
QDeclarativeListProperty<FacebookTagInterface> FacebookPhotoInterface::tags()
{
    return QDeclarativeListProperty<FacebookTagInterface>(
            this, 0,
            &FacebookPhotoInterfacePrivate::tags_append,
            &FacebookPhotoInterfacePrivate::tags_count,
            &FacebookPhotoInterfacePrivate::tags_at,
            &FacebookPhotoInterfacePrivate::tags_clear);
}

/*!
    \qmlproperty QString FacebookPhoto::name
    Holds the name (caption) of the photo
*/
QString FacebookPhotoInterface::name() const
{
    Q_D(const FacebookPhotoInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_PHOTO_NAME).toString();
}

/*!
    \qmlproperty QVariantMap FacebookPhoto::nameTags
    Holds the names of various tagged entities
*/
QVariantMap FacebookPhotoInterface::nameTags() const
{
    Q_D(const FacebookPhotoInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_PHOTO_NAMETAGS).toMap();
}

/*!
    \qmlproperty QUrl FacebookPhoto::icon
    Holds a url to the icon for the photo
*/
QUrl FacebookPhotoInterface::icon() const
{
    Q_D(const FacebookPhotoInterface);
    return QUrl(d->data().value(FACEBOOK_ONTOLOGY_PHOTO_ICON).toString());
}

/*!
    \qmlproperty QUrl FacebookPhoto::picture
    Holds a url to the picture for the photo
*/
QUrl FacebookPhotoInterface::picture() const
{
    Q_D(const FacebookPhotoInterface);
    return QUrl(d->data().value(FACEBOOK_ONTOLOGY_PHOTO_PICTURE).toString());
}

/*!
    \qmlproperty QUrl FacebookPhoto::icon
    Holds a url to the source for the photo, full size
*/
QUrl FacebookPhotoInterface::source() const
{
    Q_D(const FacebookPhotoInterface);
    return QUrl(d->data().value(FACEBOOK_ONTOLOGY_PHOTO_SOURCE).toString());
}

/*!
    \qmlproperty int FacebookPhoto::height
    Holds the height of the photo
*/
int FacebookPhotoInterface::height() const
{
    Q_D(const FacebookPhotoInterface);
    QString hStr = d->data().value(FACEBOOK_ONTOLOGY_PHOTO_HEIGHT).toString();
    bool ok = false;
    int retn = hStr.toInt(&ok);
    if (!ok)
        return -1;
    return retn;
}

/*!
    \qmlproperty int FacebookPhoto::height
    Holds the width of the photo
*/
int FacebookPhotoInterface::width() const
{
    Q_D(const FacebookPhotoInterface);
    QString wStr = d->data().value(FACEBOOK_ONTOLOGY_PHOTO_WIDTH).toString();
    bool ok = false;
    int retn = wStr.toInt(&ok);
    if (!ok)
        return -1;
    return retn;
}

/*!
    \qmlproperty QVariantMap FacebookPhoto::images
    Holds links to and metadata about scaled versions of the photo
*/
QVariantMap FacebookPhotoInterface::images() const
{
    Q_D(const FacebookPhotoInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_PHOTO_IMAGES).toMap();
}

/*!
    \qmlproperty QUrl FacebookPhoto::link
    Holds a url to the photo which may be used as an external link.
    Note that this link url contains the album identifier embedded
    within it.
*/
QUrl FacebookPhotoInterface::link() const
{
    Q_D(const FacebookPhotoInterface);
    return QUrl(d->data().value(FACEBOOK_ONTOLOGY_PHOTO_LINK).toString());
}

/*!
    \qmlproperty QVariantMap FacebookPhoto::place
    Holds information about the place associated with the photo.
    Note: this property will change in the future to return
    an object reference or location reference.
*/
QVariantMap FacebookPhotoInterface::place() const
{
    Q_D(const FacebookPhotoInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_PHOTO_PLACE).toMap(); // XXX TODO: Location/Place object reference
}

/*!
    \qmlproperty QString FacebookPhoto::createdTime
    Holds the creation time of the photo in an ISO8601-formatted string
*/
QString FacebookPhotoInterface::createdTime() const
{
    Q_D(const FacebookPhotoInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_PHOTO_CREATEDTIME).toString();
}

/*!
    \qmlproperty QString FacebookPhoto::updatedTime
    Holds the last-update time of the photo in an ISO8601-formatted string
*/
QString FacebookPhotoInterface::updatedTime() const
{
    Q_D(const FacebookPhotoInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_PHOTO_UPDATEDTIME).toString();
}

/*!
    \qmlproperty int FacebookPhoto::position
    Holds the position of the photo within the album
*/
int FacebookPhotoInterface::position() const
{
    Q_D(const FacebookPhotoInterface);
    QString posStr = d->data().value(FACEBOOK_ONTOLOGY_PHOTO_POSITION).toString();
    bool ok = false;
    int retn = posStr.toInt(&ok);
    if (!ok)
        return -1;
    return retn;
}

/*!
    \qmlproperty bool FacebookPhoto::liked
    Whether the current user has liked the photo
*/
bool FacebookPhotoInterface::liked() const
{
    Q_D(const FacebookPhotoInterface);
    return d->liked; // XXX TODO: instead of using a variable, update the data().
}
