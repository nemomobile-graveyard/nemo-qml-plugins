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

#include "facebookobjectreferenceinterface.h"
#include "facebookobjectreferenceinterface_p.h"
#include "contentiteminterface_p.h"
#include "facebookontology_p.h"

#include "facebookinterface.h"

FacebookObjectReferenceInterfacePrivate::FacebookObjectReferenceInterfacePrivate(FacebookObjectReferenceInterface *q)
    : ContentItemInterfacePrivate(q)
{
}

/*! \reimp */
void FacebookObjectReferenceInterfacePrivate::emitPropertyChangeSignals(const QVariantMap &oldData, const QVariantMap &newData)
{
    Q_Q(FacebookObjectReferenceInterface);
    QString idStr = newData.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString nameStr = newData.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();
    int typeInt = newData.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTTYPE).toInt();

    QString oldIdStr = oldData.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
    QString oldNameStr = oldData.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();
    int oldTypeInt = oldData.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTTYPE).toInt();

    if (idStr != oldIdStr)
        emit q->objectIdentifierChanged();
    if (nameStr != oldNameStr)
        emit q->objectNameChanged();
    if (typeInt != oldTypeInt)
        emit q->objectTypeChanged();

    // and call the super class implementation
    ContentItemInterfacePrivate::emitPropertyChangeSignals(oldData, newData);
}

//-------------------------------

FacebookObjectReferenceInterface::FacebookObjectReferenceInterface(QObject *parent)
    : ContentItemInterface(*(new FacebookObjectReferenceInterfacePrivate(this)), parent)
{
}

/*! \reimp */
int FacebookObjectReferenceInterface::type() const
{
    return FacebookInterface::ObjectReference;
}

/*!
    \qmlproperty QString FacebookObjectReference::objectIdentifier
    Holds the identifier of the object to which this object reference refers
*/
QString FacebookObjectReferenceInterface::objectIdentifier() const
{
    Q_D(const ContentItemInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER).toString();
}

/*!
    \qmlproperty QString FacebookObjectReference::objectName
    Holds the name of the object to which this object reference refers
*/
QString FacebookObjectReferenceInterface::objectName() const
{
    Q_D(const ContentItemInterface);
    return d->data().value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME).toString();
}

/*!
    \qmlproperty Facebook::ContentItemType FacebookObjectReference::objectType
    Holds the type of the object to which this object reference refers
*/
FacebookInterface::ContentItemType FacebookObjectReferenceInterface::objectType() const
{
    Q_D(const ContentItemInterface);
    return static_cast<FacebookInterface::ContentItemType>(d->data().value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTTYPE).toInt());
}
