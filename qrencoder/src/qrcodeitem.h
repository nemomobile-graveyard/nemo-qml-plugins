/*
 * qrcodeitem.h
 *
 * Copyright (C) 2011  Imogen Software Carsten Valdemar Munk
 *
 * Author: Tom Swindell - <t.swindell@rubyx.co.uk>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.#ifndef GRANDEWEBRUNTIME_H
 */
#ifndef QRCODEITEM_H
#define QRCODEITEM_H

#include <QDeclarativeItem>

class QRCodeItemPrivate;

class QRCodeItem : public QDeclarativeItem
{
    Q_OBJECT

    Q_ENUMS(EncodeMode)
    Q_ENUMS(ErrorLevel)

    Q_PROPERTY(int version READ version WRITE setVersion NOTIFY versionChanged)
    Q_PROPERTY(ErrorLevel level READ level WRITE setLevel NOTIFY levelChanged)
    Q_PROPERTY(EncodeMode hint READ hint WRITE setHint NOTIFY hintChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(int padding READ padding WRITE setPadding NOTIFY paddingChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QColor background READ background WRITE setBackground NOTIFY backgroundChanged)

public:
    enum EncodeMode {
        ENCODE_MODE_NULL = -1,
        ENCODE_MODE_NUMERIC  = 0,
        ENCODE_MODE_ALNUM,
        ENCODE_MODE_8BIT,
        ENCODE_MODE_KANJI
    };

    enum ErrorLevel {
        ERROR_LEVEL_LOW = 0,
        ERROR_LEVEL_MEDIUM,
        ERROR_LEVEL_QUALITY,
        ERROR_LEVEL_HIGH
    };

public:
    explicit QRCodeItem(QDeclarativeItem *parent = 0);
            ~QRCodeItem();

    int version() const;
    ErrorLevel level() const;
    EncodeMode hint() const;
    QString text() const;
    int padding() const;

    QColor color() const;
    QColor background() const;

Q_SIGNALS:
    void versionChanged(int version);
    void levelChanged(ErrorLevel level);
    void hintChanged(EncodeMode hint);
    void textChanged(const QString &text);
    void paddingChanged(int padding);
    void colorChanged(const QColor &color);
    void backgroundChanged(const QColor &background);

public Q_SLOTS:
    void setVersion(int version);
    void setLevel(ErrorLevel level);
    void setHint(EncodeMode hint);
    void setText(const QString &text);
    void setPadding(int padding);
    void setColor(const QColor &color);
    void setBackground(const QColor &color);

    QImage image(int size);
    bool   save(const QString &filename, int size = 0);

protected:
    void draw(QPainter *painter, int w, int h);
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

private:
    QRCodeItemPrivate *d;
};

#endif // QRCODEITEM_H
