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

#include "contentitemtypefilterinterface.h"
#include "filterinterface_p.h"
#include "contentiteminterface.h"

class ContentItemTypeFilterInterfacePrivate: public FilterInterfacePrivate
{
public:
    ContentItemTypeFilterInterfacePrivate();
    int type;
    int limit;
    QStringList whichFields;
};

ContentItemTypeFilterInterfacePrivate::ContentItemTypeFilterInterfacePrivate()
    : FilterInterfacePrivate(), type(0), limit(0)
{
}

// ------------------------------ ContentItemTypeFilterInterface

ContentItemTypeFilterInterface::ContentItemTypeFilterInterface(QObject *parent)
    : FilterInterface(*(new ContentItemTypeFilterInterfacePrivate), parent)
{
}

bool ContentItemTypeFilterInterface::matches(ContentItemInterface *content) const
{
    Q_D(const ContentItemTypeFilterInterface);
    if (content && content->type() > 0)
        return d->type == content->type();
    return false;
}

bool ContentItemTypeFilterInterface::matches(const QVariantMap &contentData) const
{
    Q_D(const ContentItemTypeFilterInterface);
    return d->type == contentData.value(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE).toInt();
}

int ContentItemTypeFilterInterface::type() const
{
    Q_D(const ContentItemTypeFilterInterface);
    return d->type;
}

void ContentItemTypeFilterInterface::setType(int t)
{
    Q_D(ContentItemTypeFilterInterface);
    if (d->type != t) {
        d->type = t;
        emit typeChanged();
    }
}

int ContentItemTypeFilterInterface::limit() const
{
    Q_D(const ContentItemTypeFilterInterface);
    return d->limit;
}

void ContentItemTypeFilterInterface::setLimit(int l)
{
    Q_D(ContentItemTypeFilterInterface);
    if (d->limit != l) {
        d->limit = l;
        emit limitChanged();
    }
}

QStringList ContentItemTypeFilterInterface::whichFields() const
{
    Q_D(const ContentItemTypeFilterInterface);
    return d->whichFields;
}

void ContentItemTypeFilterInterface::setWhichFields(const QStringList &wf)
{
    Q_D(ContentItemTypeFilterInterface);
    if (d->whichFields != wf) {
        d->whichFields = wf;
        emit whichFieldsChanged();
    }
}
