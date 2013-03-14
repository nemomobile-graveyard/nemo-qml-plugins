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

#include "identifiablecontentiteminterface.h"
#include "identifiablecontentiteminterface_p.h"
#include "contentiteminterface_p.h"

#include "socialnetworkinterface.h"

#include <QtDebug>

IdentifiableContentItemInterfacePrivate::IdentifiableContentItemInterfacePrivate(IdentifiableContentItemInterface *parent)
    : QObject(parent)
    , q(parent)
    , status(SocialNetworkInterface::Initializing)
    , error(SocialNetworkInterface::NoError)
    , needsReload(false)
    , currentReply(0)
{
    d = q->baseClassPrivateData();
}

IdentifiableContentItemInterfacePrivate::~IdentifiableContentItemInterfacePrivate()
{
    deleteReply();
}

QNetworkReply *IdentifiableContentItemInterfacePrivate::reply()
{
    return currentReply;
}

void IdentifiableContentItemInterfacePrivate::deleteReply()
{
    if (currentReply) {
        disconnect(currentReply);
        currentReply->deleteLater();
        currentReply = 0;
    }
}

void IdentifiableContentItemInterfacePrivate::defaultRemoveHandler()
{
    if (!reply()) {
        // if an error occurred, it might have been deleted by the error handler.
        qWarning() << Q_FUNC_INFO << "network request finished but no reply";
        return;
    }

    // Default just checks to see if the response is the text "true".
    // If it is, this handler deletes the reply(), sets the status to invalid.
    // If it isn't, this handler deletes the reply(), sets the status to error,
    // and emits responseReceived() with the given data.
    QByteArray replyData = reply()->readAll();
    deleteReply();
    bool ok = false;
    QVariantMap responseData = ContentItemInterface::parseReplyData(replyData, &ok);
    if (!ok)
        responseData.insert("response", replyData);
    if (replyData == QString(QLatin1String("true"))) {
        status = SocialNetworkInterface::Invalid; // we have been removed, so we are now invalid.
        emit q->statusChanged();
        emit q->responseReceived(responseData);
    } else {
        status = SocialNetworkInterface::Error;
        errorMessage = "remove() request returned non-true value";
        error = SocialNetworkInterface::RequestError;
        emit q->statusChanged();
        emit q->errorChanged();
        emit q->errorMessageChanged();
        emit q->responseReceived(responseData);
    }
}

void IdentifiableContentItemInterfacePrivate::defaultReloadHandler()
{
    if (!reply()) {
        // if an error occurred, it might have been deleted by the error handler.
        qWarning() << Q_FUNC_INFO << "network request finished but no reply";
        return;
    }

    // Default just checks to see if the response is a valid FB Object
    // If it is, this handler deletes the reply(), sets the status to Idle,
    // and sets the internal d->data() to the object data.
    // If it isn't, this handler deletes the reply(), sets the status to error,
    // and emits responseReceived() with the given data.
    QByteArray replyData = reply()->readAll();
    deleteReply();
    bool ok = false;
    QVariantMap responseData = ContentItemInterface::parseReplyData(replyData, &ok);
    if (!ok)
        responseData.insert("response", replyData);
    if (ok && !responseData.value("id").toString().isEmpty()) {
        if (d->data() != responseData)
            d->setData(responseData);
        status = SocialNetworkInterface::Idle;
        emit q->statusChanged();
        emit q->responseReceived(responseData);
    } else {
        status = SocialNetworkInterface::Error;
        errorMessage = "reload() request returned unidentifiable value";
        error = SocialNetworkInterface::RequestError;
        emit q->statusChanged();
        emit q->errorChanged();
        emit q->errorMessageChanged();
        emit q->responseReceived(responseData);
    }
}

void IdentifiableContentItemInterfacePrivate::defaultErrorHandler(QNetworkReply::NetworkError err)
{
    deleteReply();

    switch (err) {
        case QNetworkReply::NoError: errorMessage = QLatin1String("QNetworkReply::NoError"); break;
        case QNetworkReply::ConnectionRefusedError: errorMessage = QLatin1String("QNetworkReply::ConnectionRefusedError"); break;
        case QNetworkReply::RemoteHostClosedError: errorMessage = QLatin1String("QNetworkReply::RemoteHostClosedError"); break;
        case QNetworkReply::HostNotFoundError: errorMessage = QLatin1String("QNetworkReply::HostNotFoundError"); break;
        case QNetworkReply::TimeoutError: errorMessage = QLatin1String("QNetworkReply::TimeoutError"); break;
        case QNetworkReply::OperationCanceledError: errorMessage = QLatin1String("QNetworkReply::OperationCanceledError"); break;
        case QNetworkReply::SslHandshakeFailedError: errorMessage = QLatin1String("QNetworkReply::SslHandshakeFailedError"); break;
        case QNetworkReply::TemporaryNetworkFailureError: errorMessage = QLatin1String("QNetworkReply::TemporaryNetworkFailureError"); break;
        case QNetworkReply::ProxyConnectionRefusedError: errorMessage = QLatin1String("QNetworkReply::ProxyConnectionRefusedError"); break;
        case QNetworkReply::ProxyConnectionClosedError: errorMessage = QLatin1String("QNetworkReply::ProxyConnectionClosedError"); break;
        case QNetworkReply::ProxyNotFoundError: errorMessage = QLatin1String("QNetworkReply::ProxyNotFoundError"); break;
        case QNetworkReply::ProxyTimeoutError: errorMessage = QLatin1String("QNetworkReply::ProxyTimeoutError"); break;
        case QNetworkReply::ProxyAuthenticationRequiredError: errorMessage = QLatin1String("QNetworkReply::ProxyAuthenticationRequiredError"); break;
        case QNetworkReply::ContentAccessDenied: errorMessage = QLatin1String("QNetworkReply::ContentAccessDenied"); break;
        case QNetworkReply::ContentOperationNotPermittedError: errorMessage = QLatin1String("QNetworkReply::ContentOperationNotPermittedError"); break;
        case QNetworkReply::ContentNotFoundError: errorMessage = QLatin1String("QNetworkReply::ContentNotFoundError"); break;
        case QNetworkReply::AuthenticationRequiredError: errorMessage = QLatin1String("QNetworkReply::AuthenticationRequiredError"); break;
        case QNetworkReply::ContentReSendError: errorMessage = QLatin1String("QNetworkReply::ContentReSendError"); break;
        case QNetworkReply::ProtocolUnknownError: errorMessage = QLatin1String("QNetworkReply::ProtocolUnknownError"); break;
        case QNetworkReply::ProtocolInvalidOperationError: errorMessage = QLatin1String("QNetworkReply::ProtocolInvalidOperationError"); break;
        case QNetworkReply::UnknownNetworkError: errorMessage = QLatin1String("QNetworkReply::UnknownNetworkError"); break;
        case QNetworkReply::UnknownProxyError: errorMessage = QLatin1String("QNetworkReply::UnknownProxyError"); break;
        case QNetworkReply::UnknownContentError: errorMessage = QLatin1String("QNetworkReply::UnknownContentError"); break;
        case QNetworkReply::ProtocolFailure: errorMessage = QLatin1String("QNetworkReply::ProtocolFailure"); break;
        default: errorMessage = QLatin1String("Unknown QNetworkReply::NetworkError"); break;
    }

    error = SocialNetworkInterface::RequestError;
    status = SocialNetworkInterface::Error;

    emit q->statusChanged();
    emit q->errorChanged();
    emit q->errorMessageChanged();
}

void IdentifiableContentItemInterfacePrivate::defaultSslErrorsHandler(const QList<QSslError> &sslErrors)
{
    deleteReply();

    errorMessage = QLatin1String("SSL error: ");
    if (sslErrors.isEmpty()) {
        errorMessage += QLatin1String("unknown SSL error");
    } else {
        foreach (const QSslError &sslE, sslErrors)
            errorMessage += sslE.errorString() + QLatin1String("; ");
        errorMessage.chop(2);
    }

    error = SocialNetworkInterface::RequestError;
    status = SocialNetworkInterface::Error;

    emit q->statusChanged();
    emit q->errorChanged();
    emit q->errorMessageChanged();
}

//-------------------------------------------------

/*!
    \qmltype IdentifiableContentItem
    \instantiates IdentifiableContentItemInterface
    \inqmlmodule org.nemomobile.social 1
    \brief An IdentifiableContentItem represents an identifiable data object in a social network graph

    A data object which is identifiable is represented by an
    IdentifiableContentItem.  Instances of this sort of ContentItem
    can be used as the \c node (or central content item) in a
    SocialNetwork model.

    An IdentifiableContentItem may also have more operations
    performed on it than a non-identifiable content item.
    As these operations result in network communication, the
    IdentifiableContentItem type has \c status and \c error
    properties.

    The operations supported by default are \c reload() and
    \c remove().  More operations may be supported by derived
    types provided by specific implementations of the SocialNetwork
    interface.

    The data related to an IdentifiableContentItem are exposed
    as ContentItem instances in the model data.  For example:

    \qml
    import QtQuick 1.1
    import org.nemomobile.social 1.0

    Item {
        SocialNetwork {
            id: sn
            nodeIdentifier: "1234567"
        }

        ListView {
            model: sn
            delegate: Text { text: contentItem.data["description"] }
        }
    }
    \endqml

    Note that the preceding example will in reality fail,
    since the default SocialNetwork implementation does nothing.
    Please see the Facebook implementation documentation for
    real-world examples.
*/

IdentifiableContentItemInterface::IdentifiableContentItemInterface(QObject *parent)
    : ContentItemInterface(parent), dd(new IdentifiableContentItemInterfacePrivate(this))
{
}

IdentifiableContentItemInterface::~IdentifiableContentItemInterface()
{
    delete dd;
}

ContentItemInterfacePrivate *IdentifiableContentItemInterface::baseClassPrivateData()
{
    return d;
}

/*! \reimp */
bool IdentifiableContentItemInterface::isIdentifiable() const
{
    return true;
}

/*!
    \qmlproperty QString IdentifiableContentItem::identifier
    Holds the identifier of the identifiable content item.

    If the identifier is set after the IdentifiableContentItem
    has been initialized, the entire item will be reloaded.
    In some cases, this may cause the IdentifiableContentItem
    to become invalid (for example, if the specified identifier
    does not identify a valid object of the same type in the
    social network graph).
*/
QString IdentifiableContentItemInterface::identifier() const
{
    return dd->identifier;
}

void IdentifiableContentItemInterface::setIdentifier(const QString &id)
{
    dd->identifier = id;
    if (dd->status == SocialNetworkInterface::Initializing) {
        dd->needsReload = true;
    } else {
        reload(); // XXX TODO: allow user to set whichFields for first-time initialization?
    }
}

/*!
    \qmlproperty SocialNetwork::Status IdentifiableContentItem::status
    Holds the current status of the IdentifiableContentItem.
*/
SocialNetworkInterface::Status IdentifiableContentItemInterface::status() const
{
    return dd->status;
}

/*!
    \qmlproperty SocialNetwork::ErrorType IdentifiableContentItem::error
    Holds the most recent error which occurred during initialization or
    during network requests associated with this IdentifiableContentItem.

    Note that the \c error will not be reset if subsequent operations
    succeed.
*/
SocialNetworkInterface::ErrorType IdentifiableContentItemInterface::error() const
{
    return dd->error;
}

/*!
    \qmlproperty QString IdentifiableContentItem::errorMessage
    Holds the message associated with the most recent error which occurred
    during initialization or during network requests associated with this
    IdentifiableContentItem.

    Note that the \c errorMessage will not be reset if subsequent operations
    succeed.
*/
QString IdentifiableContentItemInterface::errorMessage() const
{
    return dd->errorMessage;
}

/*! \reimp */
void IdentifiableContentItemInterface::emitPropertyChangeSignals(const QVariantMap &oldData, const QVariantMap &newData)
{
    // most derived types will do:
    // {
    //     foreach (key, propKeys) {
    //         if (newData.value(key) != oldData.value(key)) {
    //             emit thatPropertyChanged();
    //         }
    //     }
    //     SuperClass::emitPropertyChangeSignals(oldData, newData);
    // }
    // But this one is a bit special, since if the id changed, it's a terrible error.

    // check identifier - NOTE: derived types MUST fill out this field before calling this class' implementation of emitPropertyChangeSignals.
    QString oldId = oldData.value(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID).toString();
    QString newId = newData.value(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID).toString();
    if (newId.isEmpty() && oldId.isEmpty()) {
        // this will fall through to being reported as an error due to identifier change (to empty) below.
        qWarning() << Q_FUNC_INFO << "ERROR: derived types MUST set the NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID field appropriately prior to calling the superclass emitPropertyChangeSignals() function!";
    }

    if (oldId.isEmpty())
        oldId = dd->identifier; // might have been set directly by client via icii.setIdentifier() which sets dd->identifier.

    if (oldId.isEmpty() && !newId.isEmpty()) {
        // this must be a new object created by the model.  We now have an identifier; set it and update.
        dd->identifier = newId;
        emit identifierChanged();
    } else if (newId.isEmpty() || oldId != newId) {
        // the identifier changed.  This shouldn't happen in real life.  Must be an error.
        dd->status = SocialNetworkInterface::Invalid;
        dd->error = SocialNetworkInterface::DataUpdateError;
        dd->errorMessage = QString(QLatin1String("identifier changed during data update from %1 to %2")).arg(oldId).arg(newId);
        d->s = 0;
        emit statusChanged();
        emit errorChanged();
        emit errorMessageChanged();
        emit socialNetworkChanged();
    }

    // finally, as all derived classes must do, call super class implementation.
    ContentItemInterface::emitPropertyChangeSignals(oldData, newData);
}

/*! \reimp */
void IdentifiableContentItemInterface::initializationComplete()
{
    // reload content if required.
    if (dd->needsReload) {
        dd->needsReload = false;
        dd->status = SocialNetworkInterface::Idle; // but DON'T emit, otherwise reload() will fail.
        reload(); // XXX TODO: allow specifying whichFields for first time initialization reload()?
    } else {
        dd->status = SocialNetworkInterface::Idle;
        emit statusChanged();
    }

    // finally, as all derived classes must do, call super class implementation.
    ContentItemInterface::initializationComplete();
}

/*!
    \qmlmethod bool IdentifiableContentItem::remove()
    Removes the object from the social network.

    The default implementation sends a HTTP DELETE request for the
    specified object identifier.
*/
bool IdentifiableContentItemInterface::remove()
{
    if (!request(IdentifiableContentItemInterface::Delete, dd->identifier))
        return false;

    connect(dd->reply(), SIGNAL(finished()), dd, SLOT(defaultRemoveHandler()));
    connect(dd->reply(), SIGNAL(error(QNetworkReply::NetworkError)), dd, SLOT(defaultErrorHandler(QNetworkReply::NetworkError)));
    connect(dd->reply(), SIGNAL(sslErrors(QList<QSslError>)), dd, SLOT(defaultSslErrorsHandler(QList<QSslError>)));
    return true;
}

/*!
    \qmlmethod bool IdentifiableContentItem::reload()
    Reloads the object data from the social network.  Only the fields
    specified in the \a whichFields list will be requested from the
    remote service.

    The default implementation sends a HTTP GET request for the
    specified object identifier.
*/
bool IdentifiableContentItemInterface::reload(const QStringList &whichFields)
{
    if (!request(IdentifiableContentItemInterface::Get, dd->identifier, QString(), whichFields))
        return false;

    connect(dd->reply(), SIGNAL(finished()), dd, SLOT(defaultReloadHandler()));
    connect(dd->reply(), SIGNAL(error(QNetworkReply::NetworkError)), dd, SLOT(defaultErrorHandler(QNetworkReply::NetworkError)));
    connect(dd->reply(), SIGNAL(sslErrors(QList<QSslError>)), dd, SLOT(defaultSslErrorsHandler(QList<QSslError>)));
    return true;
}

/*
    This convenience function should be called by derived types
    when they want to implement any form of network communication.

    If the request is created successfully, the state of the
    IdentifiableContentItem will change to Busy, and the function
    will return true.  The caller takes ownership of dd->reply()
    and must delete the reply via dd->deleteReply() when they are
    finished with it.
*/
bool IdentifiableContentItemInterface::request(
        IdentifiableContentItemInterface::RequestType t,
        const QString &objectIdentifier,
        const QString &extraPath,
        const QStringList &whichFields, // only valid for GET  requests
        const QVariantMap &postData,    // only valid for POST requests
        const QVariantMap &extraData)   // social-network-specific
{
    // Caller takes ownership of dd->reply() and must dd->deleteReply()
    // If request created successfully, changes state to Busy and returns true.

    if (dd->status == SocialNetworkInterface::Initializing
            || dd->status == SocialNetworkInterface::Busy
            || dd->status == SocialNetworkInterface::Invalid) {
        qWarning() << Q_FUNC_INFO << "Warning: cannot start request, because status is Initializing/Busy/Invalid";
        return false;
    }

    if (t != IdentifiableContentItemInterface::Get
            && t != IdentifiableContentItemInterface::Post
            && t != IdentifiableContentItemInterface::Delete) {
        qWarning() << Q_FUNC_INFO << "Warning: cannot start request, because request type is unknown";
        return false;
    }

    if (dd->currentReply != 0) {
        qWarning() << Q_FUNC_INFO << "Error: not Busy and yet current reply is non-null!";
        return false;
    }

    SocialNetworkInterface *sni = socialNetwork();
    if (!sni) {
        qWarning() << Q_FUNC_INFO << "Error: social network is not valid!";
        return false;
    }

    QNetworkReply *sniReply = 0;
    switch (t) {
        case IdentifiableContentItemInterface::Get:  sniReply = sni->getRequest(objectIdentifier, extraPath, whichFields, extraData); break;
        case IdentifiableContentItemInterface::Post: sniReply = sni->postRequest(objectIdentifier, extraPath, postData, extraData);   break;
        default: sniReply = sni->deleteRequest(objectIdentifier, extraPath, extraData); break;
    }

    if (sniReply) {
        dd->currentReply = sniReply;
        dd->status = SocialNetworkInterface::Busy;
        emit statusChanged();
        return true;
    }

    qWarning() << "Warning: social network was unable to create request";
    return false;
}

