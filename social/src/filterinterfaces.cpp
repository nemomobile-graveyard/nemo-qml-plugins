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

#include "filterinterface.h"
#include "contentitemtypefilterinterface.h"

#include "contentiteminterface.h"

#include <QtDebug>

// ------------------------------ FilterInterface

FilterInterface::FilterInterface(QObject *parent) : QObject(parent), m_ownedBySni(false)
{
}

FilterInterface::~FilterInterface()
{
}

bool FilterInterface::matches(ContentItemInterface *) const
{
    return false;
}

bool FilterInterface::matches(const QVariantMap &) const
{
    return false;
}

// ------------------------------ ContentItemTypeFilterInterface

ContentItemTypeFilterInterface::ContentItemTypeFilterInterface(QObject *parent)
    : FilterInterface(parent), m_type(0), m_limit(-1)
{
}

bool ContentItemTypeFilterInterface::matches(ContentItemInterface *content) const
{
    if (content && content->type() > 0)
        return m_type == content->type();
    return false;
}

bool ContentItemTypeFilterInterface::matches(const QVariantMap &contentData) const
{
    return m_type == contentData.value(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE).toInt();
}

int ContentItemTypeFilterInterface::type() const
{
    return m_type;
}

void ContentItemTypeFilterInterface::setType(int t)
{
    if (m_type != t) {
        m_type = t;
        emit typeChanged();
    }
}

int ContentItemTypeFilterInterface::limit() const
{
    return m_limit;
}

void ContentItemTypeFilterInterface::setLimit(int l)
{
    if (m_limit != l) {
        m_limit = l;
        emit limitChanged();
    }
}

QStringList ContentItemTypeFilterInterface::whichFields() const
{
    return m_whichFields;
}

void ContentItemTypeFilterInterface::setWhichFields(const QStringList &wf)
{
    if (m_whichFields != wf) {
        m_whichFields = wf;
        emit whichFieldsChanged();
    }
}
