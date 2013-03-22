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

#ifndef CONTENTITEMINTERFACE_H
#define CONTENTITEMINTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include <QtCore/QStringList>
#include <QtCore/QString>
#include <QtDeclarative/QDeclarativeParserStatus>

class IdentifiableContentItemInterface;
class SocialNetworkInterface;

#define NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE QLatin1String("org.nemomobile.social.contentitem.type")

/*
 * NOTE: if you construct one of these in C++ directly,
 * you MUST call classBegin() and componentCompleted()
 * directly after construction.
 */

/*
 * NOTE: if you derive from ContentItem, and wish to
 * provide a new type, you may not use 0 (not initialised)
 * or 1 (unknown type).  Also, no type value clash may exist
 * within that SocialNetwork.
 */

class ContentItemInterfacePrivate;
class ContentItemInterface : public QObject, public QDeclarativeParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QDeclarativeParserStatus)
    Q_PROPERTY(SocialNetworkInterface *socialNetwork READ socialNetwork WRITE setSocialNetwork NOTIFY socialNetworkChanged)
    Q_PROPERTY(int type READ type CONSTANT)
    Q_PROPERTY(QVariantMap data READ data NOTIFY dataChanged)
    Q_PROPERTY(bool isIdentifiable READ isIdentifiable CONSTANT)

public:
    explicit ContentItemInterface(QObject *parent = 0);
    virtual ~ContentItemInterface();

    // QDeclarativeParserStatus
    void classBegin();
    void componentComplete();

    // property accessors.
    SocialNetworkInterface *socialNetwork() const;
    virtual int type() const;
    QVariantMap data() const;
    virtual bool isIdentifiable() const;

    // property mutators.
    void setSocialNetwork(SocialNetworkInterface *sn);

    // invokable api
    Q_INVOKABLE IdentifiableContentItemInterface *asIdentifiable();

Q_SIGNALS:
    void socialNetworkChanged();
    void dataChanged();

protected:
    explicit ContentItemInterface(ContentItemInterfacePrivate &dd, QObject *parent = 0);
    QScopedPointer<ContentItemInterfacePrivate> d_ptr;
    bool isInitialized() const; // TODO: Is this method really useful

private:
    Q_DECLARE_PRIVATE(ContentItemInterface)
    Q_PRIVATE_SLOT(d_func(), void socialNetworkStatusChangedHandler())
    void setDataPrivate(const QVariantMap &v);
    QVariantMap dataPrivate() const;
    friend class SocialNetworkInterface;
};

Q_DECLARE_METATYPE(ContentItemInterface*)

#endif // CONTENTITEMINTERFACE_H
