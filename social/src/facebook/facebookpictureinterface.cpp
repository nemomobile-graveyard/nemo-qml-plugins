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

#include "facebookpictureinterface.h"
#include "contentiteminterface_p.h"
#include "facebookontology_p.h"

#include "facebookinterface.h"

#include <QtCore/QUrl>

FacebookPictureInterface::FacebookPictureInterface(QObject *parent)
    : ContentItemInterface(parent)
{
}

FacebookPictureInterface::~FacebookPictureInterface()
{
}

/*! \reimp */
int FacebookPictureInterface::type() const
{
    return FacebookInterface::Picture;
}

/*! \reimp */
void FacebookPictureInterface::emitPropertyChangeSignals(const QVariantMap &oldData, const QVariantMap &newData)
{
    QString srcStr = newData.value(FACEBOOK_ONTOLOGY_PICTURE_SOURCE).toString();
    QString silBool = newData.value(FACEBOOK_ONTOLOGY_PICTURE_ISSILHOUETTE).toString();

    QString oldSrcStr = newData.value(FACEBOOK_ONTOLOGY_PICTURE_SOURCE).toString();
    QString oldSilBool = newData.value(FACEBOOK_ONTOLOGY_PICTURE_ISSILHOUETTE).toString();

    if (srcStr != oldSrcStr)
        emit sourceChanged();
    if (silBool != oldSilBool)
        emit isSilhouetteChanged();

    // call the super class implementation
    ContentItemInterface::emitPropertyChangeSignals(oldData, newData);
}


/*!
    \qmlproperty QUrl FacebookPicture::source
    Holds the url to the image source of the picture.
*/
QUrl FacebookPictureInterface::source() const
{
    Q_D(const ContentItemInterface);
    return QUrl(d->data().value(FACEBOOK_ONTOLOGY_PICTURE_SOURCE).toString());
}

/*!
    \qmlproperty bool FacebookPicture::isSilhouette
    Whether the picture is a default, anonymous silhouette image
*/
bool FacebookPictureInterface::isSilhouette() const
{
    Q_D(const ContentItemInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_PICTURE_ISSILHOUETTE).toString() == QLatin1String("true");
}

