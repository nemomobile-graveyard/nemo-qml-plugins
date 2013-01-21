/*
 * Copyright (C) 2013 Jolla Ltd. <chris.adams@jollamobile.com>
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

#include "signonuicontainerinterface.h"

#include <QtDeclarative/QDeclarativeItem>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsView>
#include <QtGui/QX11EmbedContainer>

class SignOnUiContainerInterfacePrivate
{
public:
    SignOnUiContainerInterfacePrivate(SignOnUiContainerInterface *parent) : q(parent), c(0) {}

    QGraphicsView *obtainViewFromItem(QDeclarativeItem *item);
    void constructEmbedContainer(QDeclarativeItem *item);

    SignOnUiContainerInterface *q;
    QX11EmbedContainer *c;
};

QGraphicsView *SignOnUiContainerInterfacePrivate::obtainViewFromItem(QDeclarativeItem *item)
{
    if (!item)
        return 0;

    QGraphicsScene *s = item->scene();
    if (!s)
        return 0;

    QList<QGraphicsView*> views = s->views();
    if (!views.count())
        return 0;

    QGraphicsView *v = views.at(0);
    return v;
}

void SignOnUiContainerInterfacePrivate::constructEmbedContainer(QDeclarativeItem *item)
{
    // we construct it on-demand rather than during ctor, in order to ensure
    // that we've been added to the graphics scene prior to construction.
    QGraphicsView *thisView = obtainViewFromItem(item);
    QGraphicsView *parentView = obtainViewFromItem(item->parentItem());
    c = new QX11EmbedContainer(thisView ? thisView : parentView);
    q->connectSignals();
    c->hide();
}

/*
    \qmltype SignOnUiContainer
    \instantiates SignOnUiContainerInterface
    \inqmlmodule org.nemomobile.signon 1
    \brief Provides a container into which the signon-ui service can embed UI

    This type may be used by clients to define a container area
    into which the signon-ui service can embed UI elements for
    sign-on flows which require user interaction.

    To utilise the type, the client should position the item as
    any other visual object, invoke the \c show() function, and then
    pass the following values to ServiceAccountIdentity::signIn()
    via its \c sessionData parameter:

    \table
        \header
            \li Key
            \li Value
        \row
            \li WindowId
            \li The value returned by the windowId() function
        \row
            \li Embedded
            \li \c true if you wish to embed, or \c false if you want transient dialog behaviour.
    \endtable
*/

SignOnUiContainerInterface::SignOnUiContainerInterface(QDeclarativeItem *parent)
    : QDeclarativeItem(parent), d(new SignOnUiContainerInterfacePrivate(this))
{
}

SignOnUiContainerInterface::~SignOnUiContainerInterface()
{
    if (d->c) {
        d->c->close();
        d->c->deleteLater();
    }

    delete d;
}

void SignOnUiContainerInterface::connectSignals()
{
    connect(d->c, SIGNAL(clientClosed()), this, SIGNAL(embedWidgetClosed()));
    connect(d->c, SIGNAL(clientIsEmbedded()), this, SIGNAL(embedWidgetOpened()));
    connect(d->c, SIGNAL(error(QX11EmbedContainer::Error)), this, SIGNAL(embedWidgetError()));
}

/*!
    \qmlsignal void embedWidgetClosed()
    Emitted when the embedded widget created by the signon-ui service
    which is embedded into the container is closed.
*/

/*!
    \qmlsignal void embedWidgetOpened()
    Emitted when the embedded widget created by the signon-ui service
    is opened and embedded into the container.
*/

/*!
    \qmlsignal void embedWidgetError()
    Emitted if the embedded widget created by the signon-ui service
    cannot be embedded successfully into the container.
*/

/*!
    \qmlmethod SignOnUiContainer::windowId()

    Returns the native windowId of the embed container provided by
    this item, or 0 if the windowId cannot be determined.

    This value may used as the \c WindowId parameter to a sign-on
    request via the \c sessionData argument to the
    ServiceAccountIdentity::signIn() function.
*/
int SignOnUiContainerInterface::windowId()
{
    if (!d->c)
        d->constructEmbedContainer(this);

    if (d->c && d->c->winId())
        return d->c->winId();

    return 0;
}

/*!
    \qmlmethod SignOnUiContainer::show(int xOffset, int yOffset, int widthOffset, int heightOffset)

    Shows the container widget.  By default, it will be positioned and
    resized to fill the SignOnUiContainer item.  If values are provided
    for \a xOffset, \a yOffset, \a widthOffset or \a heightOffset, the
    container widget will be positioned and resized accordingly.
*/
void SignOnUiContainerInterface::show(int xOffset, int yOffset, int widthOffset, int heightOffset)
{
    if (!d->c)
        d->constructEmbedContainer(this);

    if (!d->c)
        return; // couldn't construct embed container.

    QRectF sbr = sceneBoundingRect();
    QPointF spos = scenePos();
    int x = xOffset + qRound(sbr.x()) + qRound(spos.x());
    int y = yOffset + qRound(sbr.y()) + qRound(spos.y());
    int w = widthOffset + qRound(sbr.width());
    int h = heightOffset + qRound(sbr.height());
    d->c->setGeometry(x, y, w, h);
    d->c->show();
}

/*!
    \qmlmethod SignOnUiContainer::hide()

    Hides the embed container.
*/
void SignOnUiContainerInterface::hide()
{
    if (!d->c)
        return;
    d->c->hide();
}

