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

#include "facebookcommentinterface.h"
#include "facebookcommentinterface_p.h"

#include "facebookontology_p.h"
#include "facebookobjectreferenceinterface.h"
#include "identifiablecontentiteminterface_p.h"
#include "contentiteminterface_p.h"

#include <QtDebug>

FacebookCommentInterfacePrivate::FacebookCommentInterfacePrivate(FacebookCommentInterface *parent, IdentifiableContentItemInterfacePrivate *parentData)
    : QObject(parent), dd(parentData), from(new FacebookObjectReferenceInterface(this)), action(FacebookInterfacePrivate::NoAction), liked(false)
{
}

FacebookCommentInterfacePrivate::~FacebookCommentInterfacePrivate()
{
    dd->deleteReply();
}

/*! \internal */
void FacebookCommentInterfacePrivate::finishedHandler()
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

    if (action == FacebookInterfacePrivate::LikeAction || action == FacebookInterfacePrivate::DeleteLikeAction) {
        // user initiated a "like" or "unlike" request.
        if (replyData == QString(QLatin1String("true"))) {
            dd->status = SocialNetworkInterface::Idle;
            liked = (action == FacebookInterfacePrivate::LikeAction);
            emit q->statusChanged();
            emit q->likedChanged();
            emit q->responseReceived(responseData);
        } else {
            dd->error = SocialNetworkInterface::RequestError;
            dd->errorMessage = action == FacebookInterfacePrivate::LikeAction
                ? QLatin1String("Comment like request failed")
                : QLatin1String("Comment unlike request failed");
            dd->status = SocialNetworkInterface::Error;
            emit q->statusChanged();
            emit q->errorChanged();
            emit q->errorMessageChanged();
            emit q->responseReceived(responseData);
        }
    } else {
        dd->error = SocialNetworkInterface::OtherError;
        dd->errorMessage = QLatin1String("Request finished but no action currently in progress");
        dd->status = SocialNetworkInterface::Error;
        emit q->statusChanged();
        emit q->errorChanged();
        emit q->errorMessageChanged();
        emit q->responseReceived(responseData);
    }
}

//-------------------------------

/*!
    \qmltype FacebookComment
    \instantiates FacebookCommentInterface
    \inqmlmodule org.nemomobile.social 1
    \brief A FacebookComment represents a Comment object from the Facebook OpenGraph API

    Every FacebookComment has a unique identifier, and thus a comment may be
    set as the \c node (or central content item) in the Facebook
    adapter.  The content items related to a comment include various
    likes.

    An example of usage of a FacebookComment as the node in a Facebook
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
                delegate: Label { text: "liker: " + contentItemData["name"] } // Users who liked the comment
            }
        }

        Facebook {
            id: fb
            accessToken: "your access token"    // you must supply a valid access token
            nodeIdentifier: "10150146071791729_15215233" // some valid Facebook comment id.
            filters: [ ContentItemTypeFilter { type: Facebook.Like } ]
        }

        Component.onCompleted: {
            fb.populate()
        }
    }
    \endqml

    A FacebookComment may also be used "directly" by clients, in order to
    like the comment.  An example of direct usage of the FacebookComment
    type is as follows:

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

        FacebookComment {
            id: fbc
            socialNetwork: fb
            identifier: "10150146071791729_15215233"     // some valid Facebook Comment fbid

            onStatusChanged: {
                if (status == SocialNetwork.Idle) {
                    // could like the comment
                    fbc.like()
                    // could unlike the comment
                    fbc.unlike()
                }
            }
        }

        Text {
            anchors.fill: parent
            text: fbc.message
        }
    }
    \endqml
*/

FacebookCommentInterface::FacebookCommentInterface(QObject *parent)
    : IdentifiableContentItemInterface(parent), f(new FacebookCommentInterfacePrivate(this, dd))
{
}

FacebookCommentInterface::~FacebookCommentInterface()
{
}

/*! \reimp */
int FacebookCommentInterface::type() const
{
    return FacebookInterface::Comment;
}

/*! \reimp */
bool FacebookCommentInterface::remove()
{
    return IdentifiableContentItemInterface::remove();
}

/*! \reimp */
bool FacebookCommentInterface::reload(const QStringList &whichFields)
{
    return IdentifiableContentItemInterface::reload(whichFields);
}

/*! \reimp */
void FacebookCommentInterface::emitPropertyChangeSignals(const QVariantMap &oldData, const QVariantMap &newData)
{
    // populate our new values
    QVariantMap fromMap = newData.value(FACEBOOK_ONTOLOGY_COMMENT_FROM).toMap();
    QString fromIdStr = fromMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString fromNameStr = fromMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();
    QString createdTimeStr = newData.value(FACEBOOK_ONTOLOGY_COMMENT_CREATEDTIME).toString();
    QString messageStr = newData.value(FACEBOOK_ONTOLOGY_COMMENT_MESSAGE).toString();
    int likeCountInt = newData.value(FACEBOOK_ONTOLOGY_COMMENT_LIKECOUNT).toInt();
    QString userLikesStr = newData.value(FACEBOOK_ONTOLOGY_COMMENT_LIKED).toString();
    QString typeStr = newData.value(FACEBOOK_ONTOLOGY_COMMENT_TYPE).toString();

    // populate our old values
    QVariantMap oldFromMap = oldData.value(FACEBOOK_ONTOLOGY_COMMENT_FROM).toMap();
    QString oldFromIdStr = oldFromMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString oldFromNameStr = oldFromMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();
    QString oldCreatedTimeStr = oldData.value(FACEBOOK_ONTOLOGY_COMMENT_CREATEDTIME).toString();
    QString oldMessageStr = oldData.value(FACEBOOK_ONTOLOGY_COMMENT_MESSAGE).toString();
    int oldLikeCountInt = oldData.value(FACEBOOK_ONTOLOGY_COMMENT_LIKECOUNT).toInt();
    QString oldUserLikesStr = oldData.value(FACEBOOK_ONTOLOGY_COMMENT_LIKED).toString();

    // determine if any of our properties have changed
    if (!typeStr.isEmpty() && typeStr != QLatin1String("comment"))
        qWarning() << Q_FUNC_INFO << "data does not define a comment!  type = " + typeStr;
    if (createdTimeStr != oldCreatedTimeStr)
        emit createdTimeChanged();
    if (messageStr != oldMessageStr)
        emit messageChanged();
    if (likeCountInt != oldLikeCountInt)
        emit likeCountChanged();
    if (userLikesStr != oldUserLikesStr) {
        f->liked = userLikesStr == QLatin1String("true");
        emit likedChanged();
    }

    // update the from object if required
    if (fromIdStr != oldFromIdStr || fromNameStr != oldFromNameStr) {
        QVariantMap newFromData;
        newFromData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTTYPE, FacebookInterface::User);
        newFromData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER, fromIdStr);
        newFromData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME, fromNameStr);
        qobject_cast<FacebookInterface*>(socialNetwork())->setFacebookContentItemData(f->from, newFromData);
        emit fromChanged();
    }

    // then, call super class implementation.
    IdentifiableContentItemInterface::emitPropertyChangeSignals(oldData, newData);
}

/*!
    \qmlmethod bool FacebookComment::like()
    Initiates a "like" operation on the comment.

    If the network request was started successfully, the function
    will return true and the status of the comment will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.
*/
bool FacebookCommentInterface::like()
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
    \qmlmethod bool FacebookComment::unlike()
    Initiates a "delete like" operation on the comment.

    If the network request was started successfully, the function
    will return true and the status of the comment will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.
*/
bool FacebookCommentInterface::unlike()
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
    \qmlproperty QString FacebookComment::message
    Holds the message text of the comment
*/
QString FacebookCommentInterface::message() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_COMMENT_MESSAGE).toString();
}

/*!
    \qmlproperty FacebookObjectReference *FacebookComment::from
    Holds a reference to the user or profile which uploaded this comment
*/
FacebookObjectReferenceInterface *FacebookCommentInterface::from() const
{
    return f->from;
}

/*!
    \qmlproperty QString FacebookComment::createdTime
    Holds the creation time of the comment in an ISO8601-formatted string
*/
QString FacebookCommentInterface::createdTime() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_COMMENT_CREATEDTIME).toString();
}

/*!
    \qmlproperty int FacebookPhoto::likeCount
    Holds the number of times this comment has been liked
*/
int FacebookCommentInterface::likeCount() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_COMMENT_LIKECOUNT).toInt();
}

/*!
    \qmlproperty bool FacebookPhoto::liked
    Whether the current user has liked the comment
*/
bool FacebookCommentInterface::liked() const
{
    return f->liked; // XXX TODO: instead of using a variable, update the data().
}


