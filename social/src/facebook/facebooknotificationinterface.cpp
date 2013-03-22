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

#include "facebooknotificationinterface.h"
#include "facebooknotificationinterface_p.h"

#include "facebookontology_p.h"
#include "facebookobjectreferenceinterface.h"
#include "identifiablecontentiteminterface_p.h"
#include "contentiteminterface_p.h"

#include <QtDebug>

FacebookNotificationInterfacePrivate::FacebookNotificationInterfacePrivate(FacebookNotificationInterface *q)
    : IdentifiableContentItemInterfacePrivate(q)
    , from(0)
    , to(0)
    , application(0)
    , action(FacebookInterfacePrivate::NoAction)
{
}

void FacebookNotificationInterfacePrivate::finishedHandler()
{
    Q_Q(FacebookNotificationInterface);
    if (!reply()) {
        // if an error occurred, it might have been deleted by the error handler.
        qWarning() << Q_FUNC_INFO << "network request finished but no reply";
        return;
    }

    QByteArray replyData = reply()->readAll();
    deleteReply();
    bool ok = false;
    QVariantMap responseData = ContentItemInterfacePrivate::parseReplyData(replyData, &ok);
    if (!ok)
        responseData.insert("response", replyData);

    // In the future, we will have "mark notification as read" action.
    // XXX TODO: implement "mark as read".
    error = SocialNetworkInterface::OtherError;
    errorMessage = QLatin1String("Request finished but no action currently in progress");
    status = SocialNetworkInterface::Error;
    emit q->statusChanged();
    emit q->errorChanged();
    emit q->errorMessageChanged();
    emit q->responseReceived(responseData);
}

/*! \reimp */
void FacebookNotificationInterfacePrivate::emitPropertyChangeSignals(const QVariantMap &oldData, const QVariantMap &newData)
{
    Q_Q(FacebookNotificationInterface);
    // populate our new values
    QString typeStr = newData.value(FACEBOOK_ONTOLOGY_METADATA_TYPE).toString();
    QString idStr = newData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_ID).toString();
    QVariantMap fromMap = newData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_FROM).toMap();
    QString fromIdStr = fromMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString fromNameStr = fromMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();
    QVariantMap toMap = newData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_TO).toMap();
    QString toIdStr = fromMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString toNameStr = fromMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();
    QVariantMap applicationMap = newData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_APPLICATION).toMap();
    QString applicationIdStr = fromMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString applicationNameStr = fromMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();
    QString createdTimeStr = newData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_CREATEDTIME).toString();
    QString updatedTimeStr = newData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_UPDATEDTIME).toString();
    QString titleStr = newData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_TITLE).toString();
    QString linkUrl = newData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_LINK).toString();
    QString unreadInt = newData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_UNREAD).toString();

    // populate our old values
    QVariantMap oldFromMap = oldData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_FROM).toMap();
    QString oldFromIdStr = oldFromMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString oldFromNameStr = oldFromMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();
    QVariantMap oldToMap = oldData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_TO).toMap();
    QString oldToIdStr = oldToMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString oldToNameStr = oldToMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();
    QVariantMap oldApplicationMap = oldData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_APPLICATION).toMap();
    QString oldApplicationIdStr = oldApplicationMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString oldApplicationNameStr = oldApplicationMap.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();
    QString oldCreatedTimeStr = oldData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_CREATEDTIME).toString();
    QString oldUpdatedTimeStr = oldData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_UPDATEDTIME).toString();
    QString oldTitleStr = oldData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_TITLE).toString();
    QString oldLinkUrl = oldData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_LINK).toString();
    QString oldUnreadInt = oldData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_UNREAD).toString();

    // determine if any of our properties have changed
    if (!typeStr.isEmpty() && typeStr != FACEBOOK_ONTOLOGY_NOTIFICATION)
        qWarning() << Q_FUNC_INFO << "data does not define a notification!  type = " + typeStr;
    if (typeStr.isEmpty() && !idStr.startsWith(FACEBOOK_ONTOLOGY_NOTIFICATION_ID_PREFIX))
        qWarning() << Q_FUNC_INFO << "data does not define a notification!  id = " + idStr;
    if (createdTimeStr != oldCreatedTimeStr)
        emit q->createdTimeChanged();
    if (updatedTimeStr != oldUpdatedTimeStr)
        emit q->updatedTimeChanged();
    if (titleStr != oldTitleStr)
        emit q->titleChanged();
    if (linkUrl != oldLinkUrl)
        emit q->linkChanged();
    if (unreadInt != oldUnreadInt)
        emit q->unreadChanged();


    // update the from/to/application objects if required
    if (fromIdStr != oldFromIdStr || fromNameStr != oldFromNameStr) {
        QVariantMap newFromData;
        newFromData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTTYPE, FacebookInterface::User);
        newFromData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER, fromIdStr);
        newFromData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME, fromNameStr);
        qobject_cast<FacebookInterface*>(q->socialNetwork())->setFacebookContentItemData(from, newFromData);
        emit q->fromChanged();
    }
    if (toIdStr != oldToIdStr || toNameStr != oldToNameStr) {
        QVariantMap newToData;
        newToData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTTYPE, FacebookInterface::User);
        newToData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER, toIdStr);
        newToData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME, toNameStr);
        qobject_cast<FacebookInterface*>(q->socialNetwork())->setFacebookContentItemData(to, newToData);
        emit q->toChanged();
    }
    if (applicationIdStr != oldApplicationIdStr || applicationNameStr != oldApplicationNameStr) {
        QVariantMap newApplicationData;
        newApplicationData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTTYPE, FacebookInterface::User);
        newApplicationData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER, applicationIdStr);
        newApplicationData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME, applicationNameStr);
        qobject_cast<FacebookInterface*>(q->socialNetwork())->setFacebookContentItemData(from, newApplicationData);
        emit q->applicationChanged();
    }

    // then, call super class implementation.
    QVariantMap oldDataWithId = oldData; oldDataWithId.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID, oldData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_ID));
    QVariantMap newDataWithId = newData; newDataWithId.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID, newData.value(FACEBOOK_ONTOLOGY_NOTIFICATION_ID));
    IdentifiableContentItemInterfacePrivate::emitPropertyChangeSignals(oldDataWithId, newDataWithId);
}

//-------------------------------

/*!
    \qmltype FacebookNotification
    \instantiates FacebookNotificationInterface
    \inqmlmodule org.nemomobile.social 1
    \brief A FacebookNotification represents a Notification object from the Facebook OpenGraph API

    Every FacebookNotification has a unique identifier, and thus a Notification may be
    set as the \c node (or central content item) in the Facebook
    adapter.  There are, however, no connections supported for Notifications, and
    thus clients are advised to avoid using Notification objects as nodes.  Instead,
    clients should read notifications by setting the appropriate filter on a User node.

    \qml
    import QtQuick 1.1
    import org.nemomobile.social 1.0

    Item {
        id: root
        width: 400
        height: 800

        Flickable {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right

            ListView {
                model: fb
                anchors.fill: parent
                delegate: Text { text: contentItem.title } // notification messages
            }
        }

        Facebook {
            id: fb
            accessToken: "your access token"    // you must supply a valid access token
            nodeIdentifier: "me"                // the "me" user is a "special" user id
            filters: [ ContentItemTypeFilter { type: Facebook.Notification; limit: 5 } ]
        }
    }
    \endqml

    A FacebookNotification may also be used "directly" by clients, in order to
    view details about the notification.

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

        FacebookNotification {
            id: fbn
            socialNetwork: fb
            identifier: "notif_12345678_987654321"     // some valid Facebook Notification fbid
        }

        Text {
            anchors.fill: parent
            text: fbn.title + " --> was created: " + fbn.createdTime
        }
    }
    \endqml
*/

FacebookNotificationInterface::FacebookNotificationInterface(QObject *parent)
    : IdentifiableContentItemInterface(*(new FacebookNotificationInterfacePrivate(this)), parent)
{
    Q_D(FacebookNotificationInterface);
    d->from = new FacebookObjectReferenceInterface(this);
    d->to = new FacebookObjectReferenceInterface(this);
    d->application = new FacebookObjectReferenceInterface(this);
}

/*! \reimp */
int FacebookNotificationInterface::type() const
{
    return FacebookInterface::Notification;
}

/*! \reimp */
bool FacebookNotificationInterface::remove()
{
    return IdentifiableContentItemInterface::remove();
}

/*! \reimp */
bool FacebookNotificationInterface::reload(const QStringList &whichFields)
{
    return IdentifiableContentItemInterface::reload(whichFields);
}

/*!
    \qmlproperty FacebookObjectReference *FacebookNotification::from
    Holds a reference to the person or profile whose action triggered the notification
*/
FacebookObjectReferenceInterface *FacebookNotificationInterface::from() const
{
    Q_D(const FacebookNotificationInterface);
    return d->from;
}

/*!
    \qmlproperty FacebookObjectReference *FacebookNotification::to
    Holds a reference to the person or profile to whom the notification was posted
*/
FacebookObjectReferenceInterface *FacebookNotificationInterface::to() const
{
    Q_D(const FacebookNotificationInterface);
    return d->to;
}

/*!
    \qmlproperty FacebookObjectReference *FacebookNotification::from
    Holds a reference to the application which posted the notification
*/
FacebookObjectReferenceInterface *FacebookNotificationInterface::application() const
{
    Q_D(const FacebookNotificationInterface);
    return d->application;
}

/*!
    \qmlproperty QString FacebookNotification::createdTime
    Holds the date and time that the notification was created, in ISO8601 format
*/
QString FacebookNotificationInterface::createdTime() const
{
    Q_D(const FacebookNotificationInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_NOTIFICATION_CREATEDTIME).toString();
}

/*!
    \qmlproperty QString FacebookNotification::updatedTime
    Holds the date and time that the notification was updated, in ISO8601 format
*/
QString FacebookNotificationInterface::updatedTime() const
{
    Q_D(const FacebookNotificationInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_NOTIFICATION_UPDATEDTIME).toString();
}

/*!
    \qmlproperty QString FacebookNotification::title
    Holds the title (message) of the notification
*/
QString FacebookNotificationInterface::title() const
{
    Q_D(const FacebookNotificationInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_NOTIFICATION_TITLE).toString();
}

/*!
    \qmlproperty QUrl FacebookNotification::link
    Holds a link to the original content item about which the notification was posted
*/
QUrl FacebookNotificationInterface::link() const
{
    Q_D(const FacebookNotificationInterface);
    return QUrl(d->data().value(FACEBOOK_ONTOLOGY_NOTIFICATION_LINK).toString());
}

/*!
    \qmlproperty int FacebookNotification::unread
    Will be zero if the notification has been marked as read
*/
int FacebookNotificationInterface::unread() const
{
    Q_D(const FacebookNotificationInterface);
    QString countStr = d->data().value(FACEBOOK_ONTOLOGY_NOTIFICATION_UNREAD).toString();
    bool ok = false;
    int retn = countStr.toInt(&ok);
    if (ok)
        return retn;
    return 0;
}
