/*
 * qrcodeitem.cpp
 *
 * Copyright (C) 2011-2012  Imogen Software Carsten Valdemar Munk
 *
 * Author: Tom Swindell - <t.swindell@rubyx.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
#include "common.h"
#include "qrcodeitem.h"

#include <math.h>

#ifdef HAVE_QRENCODE
#include <qrencode.h>
#endif

#include <QPainter>

class QRCodeItemPrivate
{
public:
    QRCodeItemPrivate() :
#ifdef HAVE_QRENCODE
        encoder(NULL),
#endif
        version(1),
        level(QRCodeItem::ERROR_LEVEL_MEDIUM),
        hint(QRCodeItem::ENCODE_MODE_8BIT),
        padding(0),
        color(Qt::black),
        background(Qt::white)
    { TRACE }

#ifdef HAVE_QRENCODE
    QRcode *encoder;
#endif

    int version;
    QRCodeItem::ErrorLevel level;
    QRCodeItem::EncodeMode hint;

    QString text;
    int padding;

    QColor color;
    QColor background;
};

QRCodeItem::QRCodeItem(QDeclarativeItem *parent)
    : QDeclarativeItem(parent), d(new QRCodeItemPrivate)
{
    TRACE
    this->setFlag(QDeclarativeItem::ItemHasNoContents, false);
}

QRCodeItem::~QRCodeItem()
{
    TRACE
#ifdef HAVE_QRENCODE
    delete d->encoder;
#endif
    delete this->d;
}

int QRCodeItem::version() const
{
    TRACE
    return d->version;
}

QRCodeItem::ErrorLevel QRCodeItem::level() const
{
    TRACE
    return d->level;
}

QRCodeItem::EncodeMode QRCodeItem::hint() const
{
    TRACE
    return d->hint;
}

QString QRCodeItem::text() const
{
    TRACE
    return d->text;
}

int QRCodeItem::padding() const
{
    TRACE
    return d->padding;
}

QColor QRCodeItem::color() const
{
    TRACE
    return d->color;
}

QColor QRCodeItem::background() const
{
    TRACE
    return d->background;
}

void QRCodeItem::setVersion(int version)
{
    TRACE
    d->version = version;

#ifdef HAVE_QRENCODE
    if(d->encoder)
    {
        delete d->encoder;
        d->encoder = NULL;
    }
#endif

    this->update();
    emit this->versionChanged(version);
}

void QRCodeItem::setLevel(ErrorLevel level)
{
    TRACE
    d->level = level;

#ifdef HAVE_QRENCODE
    if(d->encoder)
    {
        delete d->encoder;
        d->encoder = NULL;
    }
#endif

    this->update();
    emit this->levelChanged(level);
}

void QRCodeItem::setHint(EncodeMode hint)
{
    TRACE
    d->hint = hint;

#ifdef HAVE_QRENCODE
    if(d->encoder)
    {
        delete d->encoder;
        d->encoder = NULL;
    }
#endif

    this->update();
    emit this->hintChanged(hint);
}

void QRCodeItem::setText(const QString &text)
{
    TRACE
    d->text = text;

#ifdef HAVE_QRENCODE
    if(d->encoder)
    {
        delete d->encoder;
        d->encoder = NULL;
    }
#endif

    this->update();
    emit this->textChanged(text);
}

void QRCodeItem::setPadding(int padding)
{
    TRACE
    d->padding = padding;

    this->update();
    emit this->paddingChanged(padding);
}

void QRCodeItem::setColor(const QColor &color)
{
    TRACE
    d->color = color;
    this->update();
    emit this->colorChanged(color);
}

void QRCodeItem::setBackground(const QColor &color)
{
    TRACE
    d->background = color;
    this->update();
    emit this->backgroundChanged(color);
}

void QRCodeItem::draw(QPainter *painter, int w, int h)
{
    TRACE

#ifdef HAVE_QRENCODE
    // Check to see if we need to generate the QRCode.
    if(!d->encoder && !d->text.isNull())
    {
        d->encoder = QRcode_encodeString(d->text.toAscii(),
                                         d->version,
                                         static_cast<QRecLevel>(d->level),
                                         static_cast<QRencodeMode>(d->hint),
                                         1);
    }
#endif

    // Draw background.
    painter->setPen(QPen(d->background));
    painter->fillRect(0, 0, w, h, d->background);

#ifdef HAVE_QRENCODER
    // If we don't have an encoded code, then don't render foreground.
    if(!d->encoder) return;

    int s = (w > h ? h : w) - d->padding;
    int b = s / d->encoder->width; // Block size.

    s = b * d->encoder->width;

    // Offsets for placing image in centre.
    int x = (w - s) / 2;
    int y = (h - s) / 2;

    // Draw foreground
    painter->setPen(QPen(d->color));
    for(int i = 0; i < d->encoder->width * d->encoder->width; i++)
    {
        int c = i % d->encoder->width; // Current column index
        int r = i / d->encoder->width; // Current row index

        // If block is ON then fill in.
        if(d->encoder->data[i] & 0x01)
        {
            painter->fillRect(floor(x + b * c),
                              floor(y + b * r),
                              b, b,
                              d->color);
        }
    }
#endif
}

void QRCodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    TRACE
    this->draw(painter, floor(this->boundingRect().width()), floor(this->boundingRect().height()));
}

QImage QRCodeItem::image(int size)
{
    TRACE
#ifdef HAVE_QRENCODE
    if(d->encoder && size < d->encoder->width) size = d->encoder->width + d->padding;
#endif

    QImage result(size, size, QImage::Format_ARGB32);
    QPainter painter(&result);
    this->draw(&painter, result.width(), result.height());
    return result;
}

bool QRCodeItem::save(const QString &filename, int size)
{
    QImage image = this->image(size);
    return image.save(filename);
}
