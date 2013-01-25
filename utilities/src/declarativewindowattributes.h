/*
 * Copyright (C) 2013 Jolla Ltd.
 * Contact: John Brooks <john.brooks@jollamobile.com>
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

#ifndef DECLARATIVEWINDOWATTRIBUTES_H
#define DECLARATIVEWINDOWATTRIBUTES_H

#include <QDeclarativeItem>

/*!
   \qmlclass WindowAttributes DeclarativeWindowAttributes
   \brief Control windowsystem/compositor specific attributes

   WindowAttributes provides control over attributes specific to the
   compositor or windowing system. Properties may not have any effect
   on some systems.
 */

class DeclarativeWindowAttributes : public QDeclarativeItem
{
    Q_OBJECT
    Q_ENUMS(StackingLayer)

public:
    DeclarativeWindowAttributes(QDeclarativeItem *parent = 0);

    enum StackingLayer {
        StackNormally = 0,
        StackLockscreen = 5,
        StackHighest = 10
    };

    /*!
       \qmlproperty int stackingLayer

       Set the stacking layer of the window to place it above or below other
       special windows.
     */
    Q_PROPERTY(int stackingLayer READ stackingLayer WRITE setStackingLayer NOTIFY stackingLayerChanged)
    int stackingLayer() const { return m_stackingLayer; }
    void setStackingLayer(int layer);

    /*!
       \qmlproperty bool cannotMinimize

       Prevent the window from being minimized by gestures
     */
    Q_PROPERTY(bool cannotMinimize READ cannotMinimize WRITE setCannotMinimize NOTIFY cannotMinimizeChanged)
    bool cannotMinimize() const { return m_cannotMinimize; }
    void setCannotMinimize(bool on);

signals:
    void stackingLayerChanged();
    void cannotMinimizeChanged();

private slots:
    bool updateX11(bool delayed = true);

private:
    int m_stackingLayer;
    bool m_cannotMinimize;
};

#endif

