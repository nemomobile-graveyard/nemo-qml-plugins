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

#ifndef CONTENTITEMTYPEFILTERINTERFACE_H
#define CONTENTITEMTYPEFILTERINTERFACE_H

#include "filterinterface.h"

#include <QStringList>

class ContentItemTypeFilterInterface : public FilterInterface
{
    Q_OBJECT
    Q_PROPERTY(int type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(QStringList whichFields READ whichFields WRITE setWhichFields NOTIFY whichFieldsChanged)
    Q_PROPERTY(int limit READ limit WRITE setLimit NOTIFY limitChanged)

public:
    ContentItemTypeFilterInterface(QObject *parent = 0);
    ~ContentItemTypeFilterInterface();

    // FilterInterface
    Q_INVOKABLE bool matches(ContentItemInterface *content) const;
    Q_INVOKABLE bool matches(const QVariantMap &contentData) const;

    // properties
    int type() const;
    QStringList whichFields() const;
    int limit() const;
    void setType(int t);
    void setWhichFields(const QStringList &wf);
    void setLimit(int l);

Q_SIGNALS:
    void typeChanged();
    void whichFieldsChanged();
    void limitChanged();

private:
    int m_type;
    int m_limit;
    QStringList m_whichFields;
};

#endif // CONTENTITEMTYPEFILTERINTERFACE_H
