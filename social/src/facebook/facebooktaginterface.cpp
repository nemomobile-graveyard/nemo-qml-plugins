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

#include "facebooktaginterface.h"
#include "contentiteminterface_p.h"
#include "facebookontology_p.h"

#include "facebookinterface.h"

#include <QtDebug>

FacebookTagInterface::FacebookTagInterface(QObject *parent)
    : ContentItemInterface(parent)
{
}

FacebookTagInterface::~FacebookTagInterface()
{
}

/*! \reimp */
int FacebookTagInterface::type() const
{
    return FacebookInterface::Tag;
}

/*! \reimp */
void FacebookTagInterface::emitPropertyChangeSignals(const QVariantMap &oldData, const QVariantMap &newData)
{
    QString tidStr = newData.value(FACEBOOK_ONTOLOGY_TAG_TARGETIDENTIFIER).toString();
    QString uidStr = newData.value(FACEBOOK_ONTOLOGY_TAG_USERIDENTIFIER).toString();
    QString textStr = newData.value(FACEBOOK_ONTOLOGY_TAG_TEXT).toString();
    QString xOffset = newData.value(FACEBOOK_ONTOLOGY_TAG_XOFFSET).toString();
    QString yOffset = newData.value(FACEBOOK_ONTOLOGY_TAG_YOFFSET).toString();

    QString oldTidStr = oldData.value(FACEBOOK_ONTOLOGY_TAG_TARGETIDENTIFIER).toString();
    QString oldUidStr = oldData.value(FACEBOOK_ONTOLOGY_TAG_USERIDENTIFIER).toString();
    QString oldTextStr = oldData.value(FACEBOOK_ONTOLOGY_TAG_TEXT).toString();
    QString oldXOffset = oldData.value(FACEBOOK_ONTOLOGY_TAG_XOFFSET).toString();
    QString oldYOffset = oldData.value(FACEBOOK_ONTOLOGY_TAG_YOFFSET).toString();

    if (tidStr != oldTidStr)
        emit targetIdentifierChanged();
    if (uidStr != oldUidStr)
        emit userIdentifierChanged();
    if (textStr != oldTextStr)
        emit textChanged();
    if (xOffset != oldXOffset)
        emit xOffsetChanged();
    if (yOffset != oldYOffset)
        emit yOffsetChanged();

    if (xOffset == 0 && yOffset == 0)
        qWarning() << Q_FUNC_INFO << "TODO FIXME: xOffset and yOffset are probably sent as strings";

    // call the super class implementation
    ContentItemInterface::emitPropertyChangeSignals(oldData, newData);
}

/*!
    \internal
    Originally, I wanted to differentiate between the tagged user (target)
    and the tagger (the user who created the tag).  Not sure if the Facebook
    API supports this, however...
*/
QString FacebookTagInterface::targetIdentifier() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_TAG_TARGETIDENTIFIER).toString();
}

/*!
    \qmlproperty QString FacebookTag::userIdentifier
    Holds the identifier of the user which was tagged, if the tag
    is a user tag and not a text tag
*/
QString FacebookTagInterface::userIdentifier() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_TAG_USERIDENTIFIER).toString();
}

/*!
    \qmlproperty QString FacebookTag::text
    Holds the tag text, if the tag is a text tag and not a user tag
*/
QString FacebookTagInterface::text() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_TAG_TEXT).toString();
}

/*!
    \qmlproperty qreal FacebookTag::xOffset
    Holds the xOffset position of the tagged user or tag text in the photo
*/
qreal FacebookTagInterface::xOffset() const
{
    QString xoStr = d->data().value(FACEBOOK_ONTOLOGY_TAG_XOFFSET).toString();
    bool ok = false;
    qreal retn = xoStr.toDouble(&ok);
    if (!ok)
        return -1;
    return retn;
}

/*!
    \qmlproperty qreal FacebookTag::yOffset
    Holds the yOffset position of the tagged user or tag text in the photo
*/
qreal FacebookTagInterface::yOffset() const
{
    QString yoStr = d->data().value(FACEBOOK_ONTOLOGY_TAG_YOFFSET).toString();
    bool ok = false;
    qreal retn = yoStr.toDouble(&ok);
    if (!ok)
        return -1;
    return retn;
}


