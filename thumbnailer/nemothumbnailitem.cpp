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

#include "nemothumbnailitem.h"

#include "nemothumbnailprovider.h"
#include "nemovideothumbnailer.h"

#include <QCoreApplication>
#include <QPainter>
#include <QPixmapCache>

struct ThumbnailRequest
{
    ThumbnailRequest(NemoThumbnailItem *item, const QString &fileName, const QByteArray &cacheKey);
    ~ThumbnailRequest();

    static void enqueue(ThumbnailRequest *&queue, ThumbnailRequest *request);
    static ThumbnailRequest *dequeue(ThumbnailRequest *&queue);

    ThumbnailRequest **previous;
    ThumbnailRequest *next;
    NemoThumbnailItem *item;
    QByteArray cacheKey;
    QString fileName;
    QString mimeType;
    QSize size;
    QImage image;
    NemoThumbnailItem::FillMode fillMode;
};

ThumbnailRequest::ThumbnailRequest(
        NemoThumbnailItem *item, const QString &fileName, const QByteArray &cacheKey)
    : previous(0)
    , next(0)
    , item(item)
    , cacheKey(cacheKey)
    , fileName(fileName)
    , mimeType(item->m_mimeType)
    , size(item->m_sourceSize)
    , fillMode(item->m_fillMode)
{
}

ThumbnailRequest::~ThumbnailRequest()
{
    if (next)
        next->previous = previous;
    if (previous)
        *previous = next;
}

void ThumbnailRequest::enqueue(ThumbnailRequest *&queue, ThumbnailRequest *request)
{
    // Remove from previous list.
    if (request->next)
        request->next->previous = request->previous;
    if (request->previous)
        *request->previous = request->next;

    request->next = queue;
    if (request->next)
        request->next->previous = &request->next;
    request->previous = &queue;
    queue = request;
}

ThumbnailRequest *ThumbnailRequest::dequeue(ThumbnailRequest *&queue)
{
    ThumbnailRequest *request = queue;
    if (request) {
        if (request->next)
            request->next->previous = &queue;
        queue = request->next;
        request->previous = 0;
        request->next = 0;
    }
    return request;
}

NemoThumbnailItem::NemoThumbnailItem(QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
    , m_request(0)
    , m_priority(NormalPriority)
    , m_status(Null)
    , m_fillMode(PreserveAspectCrop)
{
    setFlag(ItemHasNoContents, false);
}

NemoThumbnailItem::~NemoThumbnailItem()
{
    if (m_request)
        NemoThumbnailLoader::instance->cancelRequest(this);
}

void NemoThumbnailItem::componentComplete()
{
    QDeclarativeItem::componentComplete();

    updateThumbnail(true);
}

QUrl NemoThumbnailItem::source() const
{
    return m_source;
}

void NemoThumbnailItem::setSource(const QUrl &source)
{
    QString strSource = source.toString();
    if (strSource.contains(QChar('%'))) {
        strSource = QUrl::fromPercentEncoding(source.toString().toLatin1());
    }

    if (m_source != strSource) {
        m_source = strSource;
        emit sourceChanged();
        updateThumbnail(true);
    }
}

QString NemoThumbnailItem::mimeType() const
{
    return m_mimeType;
}

void NemoThumbnailItem::setMimeType(const QString &mimeType)
{
    if (m_mimeType != mimeType) {
        m_mimeType = mimeType;
        emit mimeTypeChanged();
        updateThumbnail(!m_request);
    }
}

NemoThumbnailItem::Priority NemoThumbnailItem::priority() const
{
    return m_priority;
}

void NemoThumbnailItem::setPriority(Priority priority)
{
    if (m_priority != priority) {
        m_priority = priority;
        emit priorityChanged();
        if (m_request)
            NemoThumbnailLoader::instance->updateRequest(this, false);
    }
}

QSize NemoThumbnailItem::sourceSize() const
{
    return m_sourceSize;
}

void NemoThumbnailItem::setSourceSize(const QSize &size)
{
    if (m_sourceSize != size) {
        m_sourceSize = size;
        emit sourceSizeChanged();
        updateThumbnail(true);
    }
}

NemoThumbnailItem::FillMode NemoThumbnailItem::fillMode() const
{
    return m_fillMode;
}

void NemoThumbnailItem::setFillMode(FillMode mode)
{
    if (m_fillMode != mode) {
        m_fillMode = mode;
        emit fillModeChanged();
        updateThumbnail(true);
    }
}

NemoThumbnailItem::Status NemoThumbnailItem::status() const
{
    return m_status;
}

void NemoThumbnailItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (m_pixmap.isNull())
        return;

    painter->drawPixmap(QRect(0, 0, width(), height()), m_pixmap);
}

void NemoThumbnailItem::updateThumbnail(bool identityChanged)
{
    if (!isComponentComplete())
        return;

    if (m_source.isLocalFile() && !m_sourceSize.isEmpty())
        NemoThumbnailLoader::instance->updateRequest(this, identityChanged);
    else if (m_request)
        NemoThumbnailLoader::instance->cancelRequest(this);

    if (m_request && m_status != Loading) {
        m_status = Loading;
        emit statusChanged();
    } else if (!m_request && !m_pixmap.isNull()) {
        if (m_status != Ready) {
            m_status = Ready;
            emit statusChanged();
        }
        update();
    } else if ((!m_source.isValid() || m_source.isEmpty()) && m_status != Null) {
        m_status = Null;
        emit statusChanged();
    }
}

NemoThumbnailLoader *NemoThumbnailLoader::instance = 0;

NemoThumbnailLoader::NemoThumbnailLoader(QObject *parent)
    : QThread(parent)
    , m_thumbnailHighPriority(0)
    , m_thumbnailNormalPriority(0)
    , m_thumbnailLowPriority(0)
    , m_generateHighPriority(0)
    , m_generateNormalPriority(0)
    , m_generateLowPriority(0)
    , m_completedRequests(0)
    , m_quit(false)
{
    Q_ASSERT(!instance);
    instance = this;
}

NemoThumbnailLoader::~NemoThumbnailLoader()
{
    shutdown();
}

void NemoThumbnailLoader::updateRequest(NemoThumbnailItem *item, bool identityChanged)
{
    QString fileName;
    QByteArray cacheKey;
    if (identityChanged) {
        fileName = item->m_source.toLocalFile();
        cacheKey = NemoThumbnailProvider::cacheKey(fileName, item->m_sourceSize);
        if (item->m_fillMode == NemoThumbnailItem::PreserveAspectFit)
            cacheKey += 'F';

        QPixmap pixmap;
        if (QPixmapCache::find(cacheKey, &pixmap)) {
            if (item->m_request)
                cancelRequest(item);
            item->m_pixmap = pixmap;
            item->setImplicitWidth(pixmap.width());
            item->setImplicitHeight(pixmap.height());
            return;
        }
    }

    QMutexLocker locker(&m_mutex);

    if (!item->m_request) {                     // There's no current request.
        item->m_request = new ThumbnailRequest(item, fileName, cacheKey);
    } else if (item->m_request->previous) {     // The current request is pending.
        if (identityChanged) {
            item->m_request->cacheKey = cacheKey;
            item->m_request->fileName = fileName;
            item->m_request->size = item->m_sourceSize;
            item->m_request->fillMode = item->m_fillMode;
        }
        item->m_request->mimeType = item->m_mimeType;
    } else {                                    // The current request is being processed. Replace it.
        item->m_request->item = 0;
        item->m_request = identityChanged
                ? new ThumbnailRequest(item, fileName, cacheKey)
                : new ThumbnailRequest(item, item->m_request->fileName, item->m_request->cacheKey);
    }

    ThumbnailRequest::enqueue(m_thumbnailRequests[item->m_priority], item->m_request);

    m_waitCondition.wakeOne();
}

void NemoThumbnailLoader::cancelRequest(NemoThumbnailItem *item)
{
    Q_ASSERT(item->m_request);

    QMutexLocker locker(&m_mutex);
    // The only time a request doesn't belong to a list is while it is being processed.
    if (item->m_request->previous)
        delete item->m_request;
    else
        item->m_request->item = 0;
    item->m_request = 0;
}

void NemoThumbnailLoader::shutdown()
{
    if (!instance)
        return;

    {
        QMutexLocker locker(&instance->m_mutex);

        instance->m_quit = true;

        instance->m_waitCondition.wakeOne();
    }

    instance->wait();

    for (int i = 0; i < NemoThumbnailItem::PriorityCount; ++i) {
        while (instance->m_thumbnailRequests[i])
            delete instance->m_thumbnailRequests[i];
        while (instance->m_generateRequests[i])
            delete instance->m_generateRequests[i];
    }
    while (instance->m_completedRequests)
        delete instance->m_completedRequests;
}

bool NemoThumbnailLoader::event(QEvent *event)
{
    if (event->type() == QEvent::User) {
        ThumbnailRequest *completedRequest;
        {
            QMutexLocker locker(&m_mutex);
            completedRequest = m_completedRequests;
            if (completedRequest)
                completedRequest->previous = &completedRequest;
            m_completedRequests = 0;
        }

        while (completedRequest) {
            if (completedRequest->item) {
                completedRequest->item->m_request = 0;
                if (!completedRequest->image.isNull()) {
                    completedRequest->item->m_pixmap = QPixmap::fromImage(completedRequest->image);
                    QPixmapCache::insert(completedRequest->cacheKey, completedRequest->item->m_pixmap);
                    completedRequest->item->m_status = NemoThumbnailItem::Ready;
                    completedRequest->item->setImplicitWidth(completedRequest->item->m_pixmap.width());
                    completedRequest->item->setImplicitHeight(completedRequest->item->m_pixmap.height());
                    emit completedRequest->item->statusChanged();
                } else {
                    completedRequest->item->m_pixmap = QPixmap();
                    completedRequest->item->m_status = NemoThumbnailItem::Error;
                    emit completedRequest->item->statusChanged();
                }
                completedRequest->item->update();
            }
            delete completedRequest;
        }

        return true;
    } else {
        return NemoThumbnailLoader::event(event);
    }
}

void NemoThumbnailLoader::run()
{
    NemoThumbnailProvider::setupCache();

    QMutexLocker locker(&m_mutex);

    for (;;) {
        ThumbnailRequest *request = 0;
        bool tryCache = true;
        NemoThumbnailItem::Priority priority = NemoThumbnailItem::LowPriority;

        // Grab the next request in priority order.  High and normal priority thumbnails are
        // prioritized over generating any thumbnail, and low priority loading or generation
        // is deprioritized over everything else.
        if (m_quit) {
            return;
        } else if ((request = ThumbnailRequest::dequeue(m_thumbnailHighPriority))) {
            priority = NemoThumbnailItem::HighPriority;
        } else if ((request = ThumbnailRequest::dequeue(m_thumbnailNormalPriority))) {
            priority = NemoThumbnailItem::NormalPriority;
        } else if ((request = ThumbnailRequest::dequeue(m_generateHighPriority))) {
            tryCache = false;
        } else if ((request = ThumbnailRequest::dequeue(m_generateNormalPriority))) {
            tryCache = false;
        } else if ((request = ThumbnailRequest::dequeue(m_thumbnailLowPriority))) {
            priority = NemoThumbnailItem::LowPriority;
        } else if ((request = ThumbnailRequest::dequeue(m_generateLowPriority))) {
            tryCache = false;
        } else {
            m_waitCondition.wait(&m_mutex);
            continue;
        }

        Q_ASSERT(request);
        const QByteArray cacheKey = request->cacheKey;
        const QString fileName = request->fileName;
        const QString mimeType = request->mimeType;
        const QSize requestedSize = request->size;
        const bool crop = request->fillMode == NemoThumbnailItem::PreserveAspectCrop;

        locker.unlock();

        if (tryCache) {
            QImage image = NemoThumbnailProvider::loadThumbnail(fileName, cacheKey);

            locker.relock();
            if (!request->item) {
                // The request was cancelled while the thumbnail was loading, delete it now so
                // so to not spend time generating a thumbnail that won't be used.
                delete request;
            } else if (!image.isNull()) {
                request->image = image;
                if (!m_completedRequests)
                    QCoreApplication::postEvent(this, new QEvent(QEvent::User));
                ThumbnailRequest::enqueue(m_completedRequests, request);
            } else {
                ThumbnailRequest::enqueue(m_generateRequests[priority], request);
            }
        } else {
            QImage image = !mimeType.startsWith(QLatin1String("video/"), Qt::CaseInsensitive)
                    ? NemoThumbnailProvider::generateThumbnail(fileName, cacheKey, requestedSize, crop)
                    : NemoVideoThumbnailer::generateThumbnail(fileName, cacheKey, requestedSize, crop);

            locker.relock();
            request->image = image;
            if (!m_completedRequests)
                QCoreApplication::postEvent(this, new QEvent(QEvent::User));
            ThumbnailRequest::enqueue(m_completedRequests, request);
        }
    }
}
