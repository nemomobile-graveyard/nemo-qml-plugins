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

Notification::Notification(QObject *parent) :
    QObject(parent),
    replacesId_(0)
{
    connect(notificationManager(), SIGNAL(ActionInvoked(uint,QString)), this, SLOT(checkActionInvoked(uint,QString)));
    connect(notificationManager(), SIGNAL(NotificationClosed(uint,uint)), this, SLOT(checkNotificationClosed(uint,uint)));
}

QString Notification::category() const
{
    return hints.value(HINT_CATEGORY).toString();
}

void Notification::setCategory(const QString &category)
{
    if (category != this->category()) {
        hints.insert(HINT_CATEGORY, category);
        emit categoryChanged();
    }
}

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

QStringList Notification::actions() const
{
    return actions_;
}

void Notification::setActions(const QStringList &actions)
{
    if (actions_ != actions) {
        actions_ = actions;
        emit actionsChanged();
    }
}

QDateTime Notification::timestamp() const
{
    return hints.value(HINT_TIMESTAMP).toDateTime();
}

void Notification::setTimestamp(const QDateTime &timestamp)
{
    if (timestamp != this->timestamp()) {
        hints.insert(HINT_TIMESTAMP, timestamp.toString(Qt::ISODate));
        emit timestampChanged();
    }
}

QString Notification::previewSummary() const
{
    return hints.value(HINT_PREVIEW_SUMMARY).toString();
}

void Notification::setPreviewSummary(const QString &previewSummary)
{
    if (previewSummary != this->previewSummary()) {
        hints.insert(HINT_PREVIEW_SUMMARY, previewSummary);
        emit previewSummaryChanged();
    }
}

QString Notification::previewBody() const
{
    return hints.value(HINT_PREVIEW_BODY).toString();
}

void Notification::setPreviewBody(const QString &previewBody)
{
    if (previewBody != this->previewBody()) {
        hints.insert(HINT_PREVIEW_BODY, previewBody);
        emit previewBodyChanged();
    }
}

int Notification::itemCount() const
{
    return hints.value(HINT_ITEM_COUNT).toInt();
}

void Notification::setItemCount(int itemCount)
{
    if (itemCount != this->itemCount()) {
        hints.insert(HINT_ITEM_COUNT, itemCount);
        emit itemCountChanged();
    }
}

void Notification::publish()
{
    setReplacesId(notificationManager()->Notify(QFileInfo(QCoreApplication::arguments()[0]).fileName(), replacesId_, QString(), summary_, body_, actions_, hints, -1));
}

void Notification::close()
{
    if (replacesId_ != 0) {
        notificationManager()->CloseNotification(replacesId_);
        setReplacesId(0);
    }
}

void Notification::checkActionInvoked(uint id, QString actionKey)
{
    if (id == replacesId_) {
        emit actionInvoked(actionKey);
    }
}

void Notification::checkNotificationClosed(uint id, uint reason)
{
    if (id == replacesId_) {
        emit closed(reason);
        setReplacesId(0);
    }
}
