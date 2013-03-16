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

#ifndef IDENTIFIABLECONTENTITEMINTERFACE_H
#define IDENTIFIABLECONTENTITEMINTERFACE_H

#include "contentiteminterface.h"

#include "socialnetworkinterface.h"

#include <QtCore/QVariantMap>
#include <QtCore/QStringList>
#include <QtCore/QString>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QSslError>

#define NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID QLatin1String("org.nemomobile.social.contentitem.id")

/*
 * NOTE: if you construct one of these in C++ directly,
 * you MUST call classBegin() and componentCompleted()
 * directly after construction.
 */

/*
 * An IdentifiableContentItem may be used as a node in a SocialNetwork.
 * Derived types may be instantiated directly by the user if they provide
 * a valid identifier.
 *
 * Every IdentifiableContentItem may be reload()ed or remove()d, which
 * will return true if successfully able to trigger a network request.
 *
 * To upload a new IdentifiableContentItem to the social network, use
 * the invokable functions provided by derived types.  For example, the
 * FacebookAlbum type has the uploadPhoto() function.  Such functions
 * should return true if successfully able to trigger a network request.
 * When a response to the request is received, the responseReceived()
 * signal will be emitted, containing the response \c data.
 */

class IdentifiableContentItemInterfacePrivate;
class IdentifiableContentItemInterface : public ContentItemInterface
{
    Q_OBJECT

    Q_PROPERTY(QString identifier READ identifier WRITE setIdentifier NOTIFY identifierChanged)

    Q_PROPERTY(SocialNetworkInterface::Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(SocialNetworkInterface::ErrorType error READ error NOTIFY errorChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

    Q_ENUMS(SocialNetworkInterface::Status)
    Q_ENUMS(SocialNetworkInterface::ErrorType)

public:
    IdentifiableContentItemInterface(QObject *parent = 0);
    virtual ~IdentifiableContentItemInterface();

    // overrides.
    bool isIdentifiable() const;

    // properties
    QString identifier() const;
    void setIdentifier(const QString &id);

    SocialNetworkInterface::Status status() const;
    SocialNetworkInterface::ErrorType error() const;
    QString errorMessage() const;

    // invokable api.
    Q_INVOKABLE virtual bool remove();
    Q_INVOKABLE virtual bool reload(const QStringList &whichFields = QStringList());

Q_SIGNALS:
    void responseReceived(const QVariantMap &data);
    void identifierChanged();
    void statusChanged();
    void errorChanged();
    void errorMessageChanged();

protected:
    explicit IdentifiableContentItemInterface(IdentifiableContentItemInterfacePrivate &dd,
                                              QObject *parent = 0);
    virtual void emitPropertyChangeSignals(const QVariantMap &oldData, const QVariantMap &newData);
    virtual void initializationComplete();
    enum RequestType { Get = 0, Post, Delete };
    bool request(// sets dd->reply() - caller takes ownership and must call dd->deleteReply()
                 RequestType t,
                 const QString &objectIdentifier,
                 const QString &extraPath = QString(),
                 const QStringList &whichFields = QStringList(), // only valid for GET  requests
                 const QVariantMap &postData = QVariantMap(),    // only valid for POST requests
                 const QVariantMap &extraData = QVariantMap());  // social-network-specific.
private:
    Q_DECLARE_PRIVATE(IdentifiableContentItemInterface)
    Q_PRIVATE_SLOT(d_func(), void finishedHandler())
    Q_PRIVATE_SLOT(d_func(), void removeHandler())
    Q_PRIVATE_SLOT(d_func(), void reloadHandler())
    Q_PRIVATE_SLOT(d_func(), void errorHandler(QNetworkReply::NetworkError))
    Q_PRIVATE_SLOT(d_func(), void sslErrorsHandler(const QList<QSslError>&))
};

Q_DECLARE_METATYPE(IdentifiableContentItemInterface*)

#endif // IDENTIFIABLECONTENTITEMINTERFACE_H
