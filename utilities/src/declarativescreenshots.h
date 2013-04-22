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

#ifndef DECLARATIVESCREENSHOTS_H
#define DECLARATIVESCREENSHOTS_H

#include <QDeclarativeItem>

/*!
   \qmlclass Screenshots DeclarativeScreenshots
   \brief Utility component to take screenshots and save them as files.

   Screenshots can be taken of the root window or of specific components.
 */

class DeclarativeScreenshots : public QDeclarativeItem
{
    Q_OBJECT

public:
    DeclarativeScreenshots(QDeclarativeItem *parent = 0);

    /*!
       \qmlproperty string path

       Path to the directory where screenshots will be saved.
       The default is the the user's pictures directory according to
       QDesktopServices.
     */
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    QString path() const { return m_path; }
    void setPath(const QString & path);

    /*!
       \qmlproperty string name

       Base filename for screenshots. The final filename will be constructed
       from this property, a timestamp, and the image format.
       The default is "screenshot".
     */
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    QString name() const { return m_name; }
    void setName(const QString & name);

    /*!
       \qmlproperty Item target

       Component from which to take screenshots.
       Leave null to take screenshots from the desktop root window.
     */
    Q_PROPERTY(QDeclarativeItem *target READ target WRITE setTarget NOTIFY targetChanged)
    QDeclarativeItem *target() const { return m_target; }
    void setTarget(QDeclarativeItem *target);

    /*!
       \qmlproperty string format

       Image format to use when encoding the screenshots. This string will
       also be used as the filename extension. The default is "png".
    */
    Q_PROPERTY(QString format READ format WRITE setFormat NOTIFY formatChanged)
    QString format() const { return m_format; }
    void setFormat(const QString & format);

    /*!
       \qmlproperty bool formatSupported

       Read-only property that signals whether the currently selected
       image format can be used.
     */
    Q_PROPERTY(bool formatSupported READ formatSupported NOTIFY formatSupportedChanged)
    bool formatSupported() const { return m_formatSupported; }

    /*!
       \qmlmethod bool Screenshots::take()

       Takes a screenshot of the current target and saves it as a file.
       Returns true if the operation was successful, otherwise false.
     */
    Q_INVOKABLE bool take();

    /*!
       \qmlmethod bool Screenshots::take(real x, real y, real width, real height)

       Takes a screenshot of the current target, cropped to the specified
       rectangle. 
       Returns true if the operation was successful, otherwise false.
     */
    Q_INVOKABLE bool take(qreal x, qreal y, qreal height, qreal width);

signals:
    void pathChanged();
    void nameChanged();
    void targetChanged();
    void formatChanged();
    void formatSupportedChanged();

private slots:
    void targetDestroyed();

private:
    QString m_path;
    QString m_name;
    QDeclarativeItem *m_target;
    QString m_format;
    bool m_formatSupported;
};

#endif
