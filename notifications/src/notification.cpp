/*
 * Copyright (C) 2013 Jolla Ltd.
 * Contact: Vesa Halttunen <vesa.halttunen@jollamobile.com>
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
#include "notificationmanagerproxy.h"
#include "notification.h"

const char *HINT_CATEGORY = "category";
const char *HINT_ITEM_COUNT = "x-nemo-item-count";
const char *HINT_TIMESTAMP = "x-nemo-timestamp";
const char *HINT_PREVIEW_BODY = "x-nemo-preview-body";
const char *HINT_PREVIEW_SUMMARY = "x-nemo-preview-summary";

//! A proxy for accessing the notification manager
static QScopedPointer<NotificationManagerProxy> notificationManagerProxy;

NotificationManagerProxy *notificationManager()
{
    if (notificationManagerProxy.isNull()) {
        notificationManagerProxy.reset(new NotificationManagerProxy("org.freedesktop.Notifications", "/org/freedesktop/Notifications", QDBusConnection::sessionBus()));
    }
    return notificationManagerProxy.data();
}

/*!
    \qmlclass Notification
    \brief Allows notifications to be published

    The Notification class is a convenience class for using notifications
    based on the
    <a href="http://www.galago-project.org/specs/notification/0.9/">Desktop
    Notifications Specification</a> as implemented in Nemo.

    Since the Nemo implementation allows static notification parameters,
    such as icon, urgency and priority definitions, to be defined as part of
    notification category definitions, this convenience class is kept as
    simple as possible by allowing only the dynamic parameters, such as
    summary and body text, item count and timestamp to be defined. Other
    parameters should be defined in the notification category definition.

    An example of the usage of this class from a Qml application:

    \qml
    Button {
        Notification {
            id: notification
            category: "x-nemo.example"
            summary: "Notification summary"
            body: "Notification body"
            previewSummary: "Notification preview summary"
            previewBody: "Notification preview body"
            itemCount: 5
            timestamp: "2013-02-20 18:21:00"
            onClicked: console.log("Clicked")
            onClosed: console.log("Closed, reason: " + reason)
        }
        text: "Application notification, ID " + notification.replacesId
        onClicked: notification.publish()
    }
    \endqml

    After publishing the ID of the notification can be found from the
    replacesId property.
 */
Notification::Notification(QObject *parent) :
    QObject(parent),
    replacesId_(0)
{
    connect(notificationManager(), SIGNAL(ActionInvoked(uint,QString)), this, SLOT(checkActionInvoked(uint,QString)));
    connect(notificationManager(), SIGNAL(NotificationClosed(uint,uint)), this, SLOT(checkNotificationClosed(uint,uint)));
}

/*!
    \qmlproperty QString category

    The type of notification this is. Defaults to an empty string.
 */
QString Notification::category() const
{
    return hints_.value(HINT_CATEGORY).toString();
}

void Notification::setCategory(const QString &category)
{
    if (category != this->category()) {
        hints_.insert(HINT_CATEGORY, category);
        emit categoryChanged();
    }
}

/*!
    \qmlproperty uint replacesId

    The optional notification ID that this notification replaces. The server must atomically (ie with no flicker or other visual cues) replace the given notification with this one. This allows clients to effectively modify the notification while it's active. A value of value of 0 means that this notification won't replace any existing notifications. Defaults to 0.
 */
uint Notification::replacesId() const
{
    return replacesId_;
}

void Notification::setReplacesId(uint id)
{
    if (replacesId_ != id) {
        replacesId_ = id;
        emit replacesIdChanged();
    }
}

/*!
    \qmlproperty QString summary

    The summary text briefly describing the notification. Defaults to an empty string.
 */
QString Notification::summary() const
{
    return summary_;
}

void Notification::setSummary(const QString &summary)
{
    if (summary_ != summary) {
        summary_ = summary;
        emit summaryChanged();
    }
}

/*!
    \qmlproperty QString body

    The optional detailed body text. Can be empty. Defaults to an empty string.
 */
QString Notification::body() const
{
    return body_;
}

void Notification::setBody(const QString &body)
{
    if (body_ != body) {
        body_ = body;
        emit bodyChanged();
    }
}

/*!
    \qmlproperty QDateTime timestamp

    The timestamp for the notification. Should be set to the time when the event the notification is related to has occurred. Defaults to current time.
 */
QDateTime Notification::timestamp() const
{
    return hints_.value(HINT_TIMESTAMP).toDateTime();
}

void Notification::setTimestamp(const QDateTime &timestamp)
{
    if (timestamp != this->timestamp()) {
        hints_.insert(HINT_TIMESTAMP, timestamp.toString(Qt::ISODate));
        emit timestampChanged();
    }
}

/*!
    \qmlproperty QString previewSummary

    Summary text to be shown in the preview banner for the notification, if any. Defaults to an empty string.
 */
QString Notification::previewSummary() const
{
    return hints_.value(HINT_PREVIEW_SUMMARY).toString();
}

void Notification::setPreviewSummary(const QString &previewSummary)
{
    if (previewSummary != this->previewSummary()) {
        hints_.insert(HINT_PREVIEW_SUMMARY, previewSummary);
        emit previewSummaryChanged();
    }
}

/*!
    \qmlproperty QString previewBody

    Body text to be shown in the preview banner for the notification, if any. Defaults to an empty string.
 */
QString Notification::previewBody() const
{
    return hints_.value(HINT_PREVIEW_BODY).toString();
}

void Notification::setPreviewBody(const QString &previewBody)
{
    if (previewBody != this->previewBody()) {
        hints_.insert(HINT_PREVIEW_BODY, previewBody);
        emit previewBodyChanged();
    }
}

/*!
    \qmlproperty int itemCount

    The number of items represented by the notification. For example, a single notification can represent four missed calls by setting the count to 4. Defaults to 1.
 */
int Notification::itemCount() const
{
    return hints_.value(HINT_ITEM_COUNT).toInt();
}

void Notification::setItemCount(int itemCount)
{
    if (itemCount != this->itemCount()) {
        hints_.insert(HINT_ITEM_COUNT, itemCount);
        emit itemCountChanged();
    }
}

/*!
    \qmlmethod void Notification::publish()

    Publishes the notification. If replacesId is 0, it will be a new
    notification. Otherwise the existing notification with the given ID
    is updated with the new details.
*/
void Notification::publish()
{
    setReplacesId(notificationManager()->Notify(QFileInfo(QCoreApplication::arguments()[0]).fileName(), replacesId_, QString(), summary_, body_, (QStringList() << "default" << ""), hints_, -1));
}

/*!
    \qmlmethod void Notification::close()

    Closes the notification if it has been published.
*/
void Notification::close()
{
    if (replacesId_ != 0) {
        notificationManager()->CloseNotification(replacesId_);
        setReplacesId(0);
    }
}

/*!
    \qmlsignal Notification::clicked()

    Sent when the notification is clicked (its default action is invoked).
*/
void Notification::checkActionInvoked(uint id, QString actionKey)
{
    if (id == replacesId_ && actionKey == "default") {
        emit clicked();
    }
}

/*!
    \qmlsignal Notification::closed(uint reason)

    Sent when the notification has been closed.
    \a reason is 1 if the notification expired, 2 if the notification was
    dismissed by the user, 3 if the notification was closed by a call to
    CloseNotification.
*/
void Notification::checkNotificationClosed(uint id, uint reason)
{
    if (id == replacesId_) {
        emit closed(reason);
        setReplacesId(0);
    }
}
