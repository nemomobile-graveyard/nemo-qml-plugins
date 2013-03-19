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

class Notification : public QObject
{
    Q_OBJECT

public:
    Notification(QObject *parent = 0);

    Q_PROPERTY(QString category READ category WRITE setCategory NOTIFY categoryChanged)
    QString category() const;
    void setCategory(const QString &category);

    Q_PROPERTY(uint replacesId READ replacesId WRITE setReplacesId NOTIFY replacesIdChanged)
    uint replacesId() const;
    void setReplacesId(uint id);

    Q_PROPERTY(QString summary READ summary WRITE setSummary NOTIFY summaryChanged)
    QString summary() const;
    void setSummary(const QString &summary);

    Q_PROPERTY(QString body READ body WRITE setBody NOTIFY bodyChanged)
    QString body() const;
    void setBody(const QString &body);

    Q_PROPERTY(QStringList actions READ actions WRITE setActions NOTIFY actionsChanged)
    QStringList actions() const;
    void setActions(const QStringList &actions);

    Q_PROPERTY(QDateTime timestamp READ timestamp WRITE setTimestamp NOTIFY timestampChanged)
    QDateTime timestamp() const;
    void setTimestamp(const QDateTime &timestamp);

    Q_PROPERTY(QString previewSummary READ previewSummary WRITE setPreviewSummary NOTIFY previewSummaryChanged)
    QString previewSummary() const;
    void setPreviewSummary(const QString &previewSummary);

    Q_PROPERTY(QString previewBody READ previewBody WRITE setPreviewBody NOTIFY previewBodyChanged)
    QString previewBody() const;
    void setPreviewBody(const QString &previewBody);

    Q_PROPERTY(int itemCount READ itemCount WRITE setItemCount NOTIFY itemCountChanged)
    int itemCount() const;
    void setItemCount(int itemCount);

    Q_INVOKABLE void publish();
    Q_INVOKABLE void close();

signals:
    void actionInvoked(QString actionKey);
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
    QVariantHash hints_;
};

#endif // NOTIFICATION_H
