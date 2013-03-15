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

#include "facebooklikeinterface.h"
#include "contentiteminterface_p.h"
#include "facebookontology_p.h"

#include "facebookinterface.h"

FacebookLikeInterface::FacebookLikeInterface(QObject *parent)
    : ContentItemInterface(parent)
{
}

FacebookLikeInterface::~FacebookLikeInterface()
{
}

/*! \reimp */
int FacebookLikeInterface::type() const
{
    return FacebookInterface::Like;
}

/*! \reimp */
void FacebookLikeInterface::emitPropertyChangeSignals(const QVariantMap &oldData, const QVariantMap &newData)
{
    QString tidStr = newData.value(FACEBOOK_ONTOLOGY_LIKE_TARGETIDENTIFIER).toString();
    QString uidStr = newData.value(FACEBOOK_ONTOLOGY_LIKE_USERIDENTIFIER).toString();
    QString unStr = newData.value(FACEBOOK_ONTOLOGY_LIKE_USERNAME).toString();

    QString oldTidStr = newData.value(FACEBOOK_ONTOLOGY_LIKE_TARGETIDENTIFIER).toString();
    QString oldUidStr = newData.value(FACEBOOK_ONTOLOGY_LIKE_USERIDENTIFIER).toString();
    QString oldUnStr = newData.value(FACEBOOK_ONTOLOGY_LIKE_USERNAME).toString();

    if (tidStr != oldTidStr)
        emit targetIdentifierChanged();
    if (uidStr != oldUidStr)
        emit userIdentifierChanged();
    if (unStr != oldUnStr)
        emit userNameChanged();

    // call super class implementation
    ContentItemInterface::emitPropertyChangeSignals(oldData, newData);
}

/*!
    \qmlproperty QString FacebookLike::targetIdentifier
    Holds the identifier of the object which was liked
*/
QString FacebookLikeInterface::targetIdentifier() const
{
    Q_D(const ContentItemInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_LIKE_TARGETIDENTIFIER).toString();
}
/*!
    \qmlproperty QString FacebookLike::userIdentifier
    Holds the identifier of the user who liked the target object
*/
QString FacebookLikeInterface::userIdentifier() const
{
    Q_D(const ContentItemInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_LIKE_USERIDENTIFIER).toString();
}
/*!
    \qmlproperty QString FacebookLike::userName
    Holds the name of the user who liked the target object
*/
QString FacebookLikeInterface::userName() const
{
    Q_D(const ContentItemInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_LIKE_USERNAME).toString();
}


