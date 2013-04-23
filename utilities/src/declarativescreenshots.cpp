/*
 * Copyright (C) 2013 Jolla Ltd.
 * Contact: Richard Braakman <richard.braakman@jollamobile.com>
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

#include "declarativescreenshots.h"

#include <QApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QDir>
#include <QFile>
#include <QGraphicsScene>
#include <QImageWriter>
#include <QPainter>

DeclarativeScreenshots::DeclarativeScreenshots(QDeclarativeItem *parent)
  : QDeclarativeItem(parent), m_name("screenshot"), m_target(0),
    m_format("png"), m_formatSupported(true) {
    m_path = QDesktopServices::storageLocation(
                QDesktopServices::PicturesLocation);
}

void DeclarativeScreenshots::setPath(const QString & path) {
    if (path == m_path)
        return;
    m_path = path;
    emit pathChanged();
}

void DeclarativeScreenshots::setName(const QString & name) {
    if (name == m_name)
        return;
    m_name = name;
    emit nameChanged();
}

void DeclarativeScreenshots::setTarget(QDeclarativeItem *target) {
    if (target == m_target)
        return;

    if (m_target)
        disconnect(m_target, SIGNAL(destroyed()),
                   this, SLOT(targetDestroyed()));
    if (target)
        connect(target, SIGNAL(destroyed()),
                this, SLOT(targetDestroyed()));

    m_target = target;
    emit targetChanged();
}

// This is connected to m_target's destroyed() signal.
// The connection is maintained by setTarget().
// This logic effectively makes m_target a weak reference.
void DeclarativeScreenshots::targetDestroyed() {
    setTarget(0);
}

void DeclarativeScreenshots::setFormat(const QString & format) {
    if (format == m_format)
        return;
    bool old_supported = m_formatSupported;

    // Set both m_format and m_formatSupported before sending any signals,
    // so that receivers see a consistent state.

    m_format = format;
    m_formatSupported = false;
    QByteArray cmpformat = format.toAscii().toLower();
    Q_FOREACH(QByteArray fmt, QImageWriter::supportedImageFormats()) {
        if (fmt.toLower() == cmpformat) {
            m_formatSupported = true;
            break;
        }
    }

    emit formatChanged();
    if (m_formatSupported != old_supported)
        emit formatSupportedChanged();
}

bool DeclarativeScreenshots::take(qreal x, qreal y, qreal width, qreal height) {
    QString timestamp = QDateTime::currentDateTimeUtc()
                        .toString("-yyyyMMdd-hhmmsszzz.");
    QPixmap snapshot;

    if (m_target) {
        QGraphicsScene *scene = m_target->scene();
        if (!scene)
            return false;

        if (width < 0)
            width = m_target->width() - x;
        if (height < 0)
            height = m_target->height() - y;

        QRectF mappedRect = m_target->mapRectToScene(x, y, width, height);
        QRectF targetRect(0, 0, mappedRect.width(), mappedRect.height());
        snapshot = QPixmap(mappedRect.size().toSize());
        snapshot.fill(Qt::transparent);

        QPainter painter(&snapshot);
        scene->render(&painter, targetRect, mappedRect);
    } else {
        // If there's no target item, then take an actual screenshot of the
        // desktop root window.
        snapshot = QPixmap::grabWindow(QApplication::desktop()->winId(),
                                       x, y, width, height);
    }

    QString pathname = QDir(m_path).filePath(m_name + timestamp + m_format);
    return snapshot.save(pathname);
}

bool DeclarativeScreenshots::take() {
    return take(0, 0, -1, -1);
}
