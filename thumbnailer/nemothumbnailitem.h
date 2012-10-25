/*
 * Copyright (C) 2012 Jolla Ltd
 * Contact: Andrew den Exter <andrew.den.exter@jollamobile.com>
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

#ifndef NEMOTHUMBNAILITEM_H
#define NEMOTHUMBNAILITEM_H

#include <QtCore/qmutex.h>
#include <QtCore/qthread.h>
#include <QtCore/qwaitcondition.h>
#include <QtDeclarative/qdeclarativeitem.h>

struct ThumbnailRequest;

class NemoThumbnailItem : public QDeclarativeItem
{
    Q_OBJECT
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QString mimeType READ mimeType WRITE setMimeType NOTIFY mimeTypeChanged)
    Q_PROPERTY(QSize sourceSize READ sourceSize WRITE setSourceSize NOTIFY sourceSizeChanged)
    Q_PROPERTY(Priority priority READ priority WRITE setPriority NOTIFY priorityChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_ENUMS(Priority)
    Q_ENUMS(Status)
public:
    enum Priority
    {
        HighPriority,
        NormalPriority,
        LowPriority
    };

    enum
    {
        PriorityCount = 3
    };

    enum Status
    {
        Null,
        Ready,
        Loading,
        Error
    };

    explicit NemoThumbnailItem(QDeclarativeItem *parent = 0);
    ~NemoThumbnailItem();

    void componentComplete();

    QUrl source() const;
    void setSource(const QUrl &source);

    QString mimeType() const;
    void setMimeType(const QString &mimeType);

    QSize sourceSize() const;
    void setSourceSize(const QSize &size);

    Priority priority() const;
    void setPriority(Priority priority);

    Status status() const;

    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

Q_SIGNALS:
    void sourceChanged();
    void mimeTypeChanged();
    void sourceSizeChanged();
    void priorityChanged();
    void statusChanged();

private:
    Q_DISABLE_COPY(NemoThumbnailItem)

    void updateThumbnail(bool identityChanged);

    ThumbnailRequest *m_request;
    QUrl m_source;
    QString m_mimeType;
    QSize m_sourceSize;
    QPixmap m_pixmap;
    Priority m_priority;
    Status m_status;

    friend struct ThumbnailRequest;
    friend class NemoThumbnailLoader;
};

class NemoThumbnailLoader : public QThread
{
public:
    explicit NemoThumbnailLoader(QObject *parent = 0);
    ~NemoThumbnailLoader();

    void updateRequest(NemoThumbnailItem *item, bool identityChanged);
    void cancelRequest(NemoThumbnailItem *item);

    static void shutdown();

    static NemoThumbnailLoader *instance;

protected:
    bool event(QEvent *event);
    void run();

private:
    union {
        struct {
            ThumbnailRequest *m_thumbnailRequests[NemoThumbnailItem::PriorityCount];
            ThumbnailRequest *m_generateRequests[NemoThumbnailItem::PriorityCount];
        };
        struct {
            ThumbnailRequest *m_thumbnailHighPriority;
            ThumbnailRequest *m_thumbnailNormalPriority;
            ThumbnailRequest *m_thumbnailLowPriority;
            ThumbnailRequest *m_generateHighPriority;
            ThumbnailRequest *m_generateNormalPriority;
            ThumbnailRequest *m_generateLowPriority;
        };
    };
    ThumbnailRequest *m_completedRequests;

    QMutex m_mutex;
    QWaitCondition m_waitCondition;
    bool m_quit;
};

#endif
