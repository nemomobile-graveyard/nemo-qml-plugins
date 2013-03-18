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

#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QStringList>
#include <QDateTime>
#include <QVariantHash>

/*!
    \qmlclass Notification
    \brief Allows notifications to be published

    The Notification class is a convenience class for using notifiations
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
            actions: [ "default", "Default action" ]
            onActionInvoked: console.log("Action invoked: " + actionKey)
            onClosed: console.log("Closed, reason: " + reason)
        }
        text: "Application notification, ID " + notification.replacesId
        onClicked: notification.publish()
    }
 */
class Notification : public QObject
{
    Q_OBJECT

public:
    Notification(QObject *parent = 0);

    /*!
        \qmlproperty QString category

        The type of notification this is. Defaults to an empty string.
     */
    Q_PROPERTY(QString category READ category WRITE setCategory NOTIFY categoryChanged)
    QString category() const;
    void setCategory(const QString &category);

    /*!
        \qmlproperty uint replacesId

        The optional notification ID that this notification replaces. The server must atomically (ie with no flicker or other visual cues) replace the given notification with this one. This allows clients to effectively modify the notification while it's active. A value of value of 0 means that this notification won't replace any existing notifications. Defaults to 0.
     */
    Q_PROPERTY(uint replacesId READ replacesId WRITE setReplacesId NOTIFY replacesIdChanged)
    uint replacesId() const;
    void setReplacesId(uint id);

    /*!
        \qmlproperty QString summary

        The summary text briefly describing the notification. Defaults to an empty string.
     */
    Q_PROPERTY(QString summary READ summary WRITE setSummary NOTIFY summaryChanged)
    QString summary() const;
    void setSummary(const QString &summary);

    /*!
        \qmlproperty QString body

        The optional detailed body text. Can be empty. Defaults to an empty string.
     */
    Q_PROPERTY(QString body READ body WRITE setBody NOTIFY bodyChanged)
    QString body() const;
    void setBody(const QString &body);

    /*!
        \qmlproperty QStringList actions

        Actions are sent over as a list of pairs. Each even element in the list (starting at index 0) represents the identifier for the action. Each odd element in the list is the localized string that will be displayed to the user. Defaults to an empty list.
     */
    Q_PROPERTY(QStringList actions READ actions WRITE setActions NOTIFY actionsChanged)
    QStringList actions() const;
    void setActions(const QStringList &actions);

    /*!
        \qmlproperty QDateTime timestamp

        The timestamp for the notification. Should be set to the time when the event the notification is related to has occurred. Defaults to current time.
     */
    Q_PROPERTY(QDateTime timestamp READ timestamp WRITE setTimestamp NOTIFY timestampChanged)
    QDateTime timestamp() const;
    void setTimestamp(const QDateTime &timestamp);

    /*!
        \qmlproperty QString previewSummary

        Summary text to be shown in the preview banner for the notification, if any. Defaults to an empty string.
     */
    Q_PROPERTY(QString previewSummary READ previewSummary WRITE setPreviewSummary NOTIFY previewSummaryChanged)
    QString previewSummary() const;
    void setPreviewSummary(const QString &previewSummary);

    /*!
        \qmlproperty QString previewBody

        Body text to be shown in the preview banner for the notification, if any. Defaults to an empty string.
     */
    Q_PROPERTY(QString previewBody READ previewBody WRITE setPreviewBody NOTIFY previewBodyChanged)
    QString previewBody() const;
    void setPreviewBody(const QString &previewBody);

    /*!
        \qmlproperty int itemCount

        The number of items represented by the notification. For example, a single notification can represent four missed calls by setting the count to 4. Defaults to 1.
     */
    Q_PROPERTY(int itemCount READ itemCount WRITE setItemCount NOTIFY itemCountChanged)
    int itemCount() const;
    void setItemCount(int itemCount);

    /*!
        \qmlmethod void Notification::publish()

        Publishes the notification. If replacesId is 0, it will be a new
        notification. Otherwise the existing notification with the given ID
        is updated with the new details.
    */
    Q_INVOKABLE void publish();

    /*!
        \qmlmethod void Notification::close()

        Closes the notification if it has been published.
    */
    Q_INVOKABLE void close();

signals:
    /*!
        \qmlsignal Notification::actionInvoked(QString actionKey)

        Sent when one of the actions defined in the actions list has been
        invoked. \a actionKey is the key for the invoked action.
    */
    void actionInvoked(QString actionKey);

    /*!
        \qmlsignal Notification::closed(uint reason)

        Sent when the notification has been closed.
        \a reason is 1 if the notification expired, 2 if the notification was
        dismissed by the user, 3 if the notification was closed by a call to
        CloseNotification.
    */
    void closed(uint reason);

    void categoryChanged();
    void replacesIdChanged();
    void summaryChanged();
    void bodyChanged();
    void actionsChanged();
    void timestampChanged();
    void previewSummaryChanged();
    void previewBodyChanged();
    void itemCountChanged();

private slots:
    void checkActionInvoked(uint id, QString actionKey);
    void checkNotificationClosed(uint id, uint reason);

private:
    uint replacesId_;
    QString summary_;
    QString body_;
    QStringList actions_;
    QVariantHash hints;
};

#endif // NOTIFICATION_H
