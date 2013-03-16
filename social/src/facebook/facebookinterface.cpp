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

#include "facebookinterface.h"
#include "facebookinterface_p.h"
#include "facebookontology_p.h"
#include "socialnetworkinterface_p.h"

#include "contentiteminterface.h"
#include "identifiablecontentiteminterface.h"
#include "contentitemtypefilterinterface.h"

#include "facebookcommentinterface.h"
#include "facebookphotointerface.h"
#include "facebookalbuminterface.h"
#include "facebookuserinterface.h"
#include "facebooklikeinterface.h"
#include "facebookobjectreferenceinterface.h"
#include "facebookpictureinterface.h"
#include "facebooktaginterface.h"
#include "facebooknotificationinterface.h"

#include <QtCore/QVariantMap>
#include <QtCore/QStringList>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QByteArray>
#include <QtNetwork/QNetworkReply>

#include <QtDebug>

QVariant FACEBOOK_DEBUG_VALUE_STRING_FROM_DATA(const QString &key, const QVariantMap &data)
{
    QVariant val = data.value(key);
    if (val.type() == QVariant::List)
        return val.toStringList();
    else if (val.type() != QVariant::Map)
        return val;
    QVariant dataVal = val.toMap().value(FACEBOOK_ONTOLOGY_CONNECTIONS_DATA);
    if (dataVal.type() == QVariant::List)
        return QVariant(QString(QLatin1String("%1 data entries")).arg(dataVal.toList().count()));
    if (dataVal.type() == QVariant::Map)
        return QVariant(QString(QLatin1String("... some other map")));
    return dataVal;
}

FacebookInterfacePrivate::FacebookInterfacePrivate(FacebookInterface *q)
    : SocialNetworkInterfacePrivate(q)
    , populatePending(false)
    , populateDataForUnseenPending(false)
    , continuationRequestActive(false)
    , outOfBandConnectionsLimit(-1)
    , internalStatus(FacebookInterfacePrivate::Idle)
    , currentReply(0)
{
}

FacebookInterfacePrivate::~FacebookInterfacePrivate()
{
}

/*!
    \internal

    Attempt to detect which type of object the given \a data represents.
    This function makes use of some fairly inexact heuristics, and only
    supports comments, albums, photos, and users.
*/
int FacebookInterfacePrivate::detectTypeFromData(const QVariantMap &data) const
{
    // attempt to detect the type directly from the Facebook metadata field
    if (data.contains(FACEBOOK_ONTOLOGY_METADATA)
            && data.value(FACEBOOK_ONTOLOGY_METADATA).type() == QVariant::Map
            && !data.value(FACEBOOK_ONTOLOGY_METADATA_TYPE).toString().isEmpty()) {
        QString typeStr = data.value(FACEBOOK_ONTOLOGY_METADATA).toMap().value(FACEBOOK_ONTOLOGY_METADATA_TYPE).toString();
        if (typeStr == FACEBOOK_ONTOLOGY_USER)
            return FacebookInterface::User;
        else if (typeStr == FACEBOOK_ONTOLOGY_ALBUM)
            return FacebookInterface::Album;
        else if (typeStr == FACEBOOK_ONTOLOGY_COMMENT)
            return FacebookInterface::Comment;
        else if (typeStr == FACEBOOK_ONTOLOGY_PHOTO)
            return FacebookInterface::Photo;

        qWarning() << Q_FUNC_INFO << "Unsupported type:" << typeStr;
        return FacebookInterface::Unknown;
    }

    // it's possible that we've already tagged the type already
    if (data.contains(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE)) {
        FacebookInterface::ContentItemType taggedType = static_cast<FacebookInterface::ContentItemType>(data.value(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE).toInt());
        if (taggedType > FacebookInterface::Unknown) {
            return taggedType;
        }
    }

    qWarning() << Q_FUNC_INFO << "No type information available in object metadata - attempting to heuristically detect type";
    if (data.value(FACEBOOK_ONTOLOGY_COMMENT_MESSAGE).isValid() && data.value(FACEBOOK_ONTOLOGY_COMMENT_LIKECOUNT).isValid())
        return FacebookInterface::Comment;
    else if (data.value(FACEBOOK_ONTOLOGY_ALBUM_PRIVACY).isValid() && data.value(FACEBOOK_ONTOLOGY_ALBUM_CANUPLOAD).isValid())
        return FacebookInterface::Album;
    else if (data.value(FACEBOOK_ONTOLOGY_PHOTO_TAGS).isValid() && data.value(FACEBOOK_ONTOLOGY_PHOTO_SOURCE).isValid())
        return FacebookInterface::Photo;
    else if (data.value(FACEBOOK_ONTOLOGY_USER_FIRSTNAME).isValid() || data.value(FACEBOOK_ONTOLOGY_USER_GENDER).isValid())
        return FacebookInterface::User;

    qWarning() << Q_FUNC_INFO << "Unable to heuristically detect type!";
    foreach (const QString &datakey, data.keys())
        qWarning() << "        " << datakey << " = " << FACEBOOK_DEBUG_VALUE_STRING_FROM_DATA(datakey, data);
    return FacebookInterface::Unknown;
}

/*! \internal */
QUrl FacebookInterfacePrivate::requestUrl(const QString &objectId, const QString &extraPath, const QStringList &whichFields, const QVariantMap &extraData)
{
    QString joinedFields = whichFields.join(QLatin1String(","));
    QList<QPair<QString, QString> > queryItems;
    if (!accessToken.isEmpty())
        queryItems.append(qMakePair<QString, QString>(QLatin1String("access_token"), accessToken));
    if (!whichFields.isEmpty())
        queryItems.append(qMakePair<QString, QString>(QLatin1String("fields"), joinedFields));
    QStringList extraDataKeys = extraData.keys();
    foreach (const QString &edk, extraDataKeys)
        queryItems.append(qMakePair<QString, QString>(edk, extraData.value(edk).toString()));

    QUrl retn;
    retn.setScheme("https");
    retn.setHost("graph.facebook.com");
    retn.setPath(objectId + QLatin1String("/") + extraPath);
    retn.setQueryItems(queryItems);
    return retn;
}

/*! \internal */
QNetworkReply *FacebookInterfacePrivate::uploadImage(const QString &objectId, const QString &extraPath, const QVariantMap &data, const QVariantMap &extraData)
{
    Q_Q(FacebookInterface);
    Q_UNUSED(extraData); // XXX TODO: privacy passed via extraData?

    // the implementation code for this function is taken from the transfer engine
    QNetworkRequest request;
    QUrl url("https://graph.facebook.com");
    QString path = objectId;
    if (!extraPath.isEmpty())
        path += QLatin1String("/") + extraPath;
    url.setPath(path);
    request.setUrl(url);

    QString multipartBoundary = QLatin1String("-------Sska2129ifcalksmqq3");
    QString filePath = data.value("source").toUrl().toLocalFile();
    QString mimeType = QLatin1String("image/jpeg");
    if (filePath.endsWith("png"))
        mimeType = QLatin1String("image/png"); // XXX TODO: more mimetypes?  better way to do this?

    QFile f(filePath, q);
    if(!f.open(QIODevice::ReadOnly)){
        qWarning() << Q_FUNC_INFO << "Error opening image file:" << filePath;
        return 0;
    }

    QByteArray imageData(f.readAll());
    f.close();

    QFileInfo info(filePath);

    // Fill in the image data first
    QByteArray postData;
    postData.append("--"+multipartBoundary+"\r\n");
    postData.append("Content-Disposition: form-data; name=\"access_token\"\r\n\r\n");
    postData.append(accessToken);
    postData.append("\r\n");

    // Actually the title isn't used
    postData.append("--"+multipartBoundary+"\r\n");
    postData.append("Content-Disposition: form-data; name=\"message\"\r\n\r\n");
    postData.append(data.value("message").toString());
    postData.append("\r\n");

    postData.append("--"+multipartBoundary+"\r\n");
    postData.append("Content-Disposition: form-data; name=\"name\"; filename=\""+info.fileName()+"\"\r\n");
    postData.append("Content-Type:"+mimeType+"\r\n\r\n");
    postData.append(imageData);
    postData.append("\r\n");

    postData.append("--"+multipartBoundary+"\r\n");
    postData.append("Content-Disposition: form-data; name=\"privacy\"\r\n\r\n");
    postData.append(QString("{\'value\':\'ALL_FRIENDS\'}"));
    postData.append("\r\n");
    postData.append("--"+multipartBoundary+"\r\n");

    // Header required
    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    request.setRawHeader("Accept-Language", "en-us,en;q=0.5");
    request.setRawHeader("Accept-Encoding", "gzip,deflate");
    request.setRawHeader("Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7");
    request.setRawHeader("Keep-Alive", "300");
    request.setRawHeader("Connection", "keep-alive");
    request.setRawHeader("Content-Type",QString("multipart/form-data; boundary="+multipartBoundary).toAscii());
    request.setHeader(QNetworkRequest::ContentLengthHeader, postData.size());

    return qnam->post(request, postData);
}

/*! \internal */
void FacebookInterfacePrivate::finishedHandler()
{
    Q_Q(FacebookInterface);
    if (!currentReply) {
        // if an error occurred, it might have been deleted by the error handler.
        qWarning() << Q_FUNC_INFO << "network request finished but no reply!";
        return;
    }

    QByteArray replyData = currentReply->readAll();
    QUrl requestUrl = currentReply->request().url();
    QVariant specialLimitVar = currentReply->property("specialLimit");
    deleteReply();
    bool ok = false;
    QVariantMap responseData = ContentItemInterface::parseReplyData(replyData, &ok);
    if (!ok) {
        responseData.insert("response", replyData);
        error = SocialNetworkInterface::RequestError;
        errorMessage = QLatin1String("Error populating node: response is invalid.  Perhaps the requested object id was incorrect?  Response: ") + QString::fromLatin1(replyData.constData());
        status = SocialNetworkInterface::Error;
        return;
    }

    // some forms of requests require manual limit checking because
    // Facebook refuses to observe "limit" parameters in some cases
    // eg, Notifications.  Its paging API is completely broken too.
    int specialLimit = -1;
    if (specialLimitVar.isValid()) {
        specialLimit = specialLimitVar.toInt();
    }

    if (internalStatus == FacebookInterfacePrivate::PopulatingUnseenNode) {
        // This one is tricky, because we don't know the type of the current node.
        q->continuePopulateDataForUnseenNode(responseData);
    } else if (internalStatus == FacebookInterfacePrivate::PopulatingSeenNode) {
        // This one should be simpler because each of the requested fields/connections is a property.
        outOfBandConnectionsLimit = specialLimit;
        q->continuePopulateDataForSeenNode(responseData, requestUrl);
    } else {
        qWarning() << Q_FUNC_INFO << "Error: network reply finished while in unexpectant state!  Received:" << responseData;
    }
}

/*! \internal */
void FacebookInterfacePrivate::errorHandler(QNetworkReply::NetworkError err)
{
    Q_Q(FacebookInterface);
    qWarning() << Q_FUNC_INFO << "Error: network error occurred:" << err;

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

    if ((internalStatus == FacebookInterfacePrivate::PopulatingUnseenNode
            || internalStatus == FacebookInterfacePrivate::PopulatingSeenNode)
            && repopulatingCurrentNode) {
        // failed repopulating, either at "get node" step, or at "get related data" step.
        repopulatingCurrentNode = false;
    }

    if (continuationRequestActive) {
        // failed during a continuation request.  This shouldn't be a huge deal,
        // since we have been populating the cache as we received more data anyway
        continuationRequestActive = false;
    }

    emit q->statusChanged();
    emit q->errorChanged();
    emit q->errorMessageChanged();
}

/*! \internal */
void FacebookInterfacePrivate::sslErrorsHandler(const QList<QSslError> &errs)
{
    Q_Q(FacebookInterface);
    errorMessage = QLatin1String("SSL error: ");
    if (errs.isEmpty()) {
        errorMessage += QLatin1String("unknown SSL error");
    } else {
        foreach (const QSslError &sslE, errs)
            errorMessage += sslE.errorString() + QLatin1String("; ");
        errorMessage.chop(2);
    }

    error = SocialNetworkInterface::RequestError;
    status = SocialNetworkInterface::Error;

    emit q->statusChanged();
    emit q->errorChanged();
    emit q->errorMessageChanged();
}

/*! \internal */
void FacebookInterfacePrivate::deleteReply()
{
    if (currentReply) {
//        disconnect(currentReply); // Is this needed ?
        currentReply->deleteLater();
        currentReply = 0;
    }
}

//----------------------------------------------------------

/*!
    \qmltype Facebook
    \instantiates FacebookInterface
    \inqmlmodule org.nemomobile.social 1
    \brief Implements the SocialNetwork interface for the Facebook service.

    The Facebook type is an implementation of the SocialNetwork interface
    specifically for the Facebook social network service.
    It provides access to graph objects such as users, albums and photographs,
    and allows operations such as "like" and "comment".

    Clients should provide an \c accessToken to use the adapter.  The access
    token may be retrieved via the org.nemomobile.signon adapters, or from
    another OAuth2 implementation.

    An example of usage is as follows:

    \qml
    import QtQuick 1.1
    import org.nemomobile.social 1.0

    Item {
        id: root
        width: 400
        height: 800

        Flickable {
            anchors.top: parent.verticalCenter
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right

            ListView {
                model: fb
                anchors.fill: parent
                delegate: Label { text: "id: " + contentItemIdentifier }
            }
        }

        Facebook {
            id: fb
            accessToken: "your access token"    // you must supply a valid access token
            nodeIdentifier: "10150146071791729" // some valid Facebook album id.
            filters: [ ContentItemTypeFilter { type: Facebook.Photo } ]
        }

        FacebookPhoto {
            id: fbph
            socialNetwork: fb
            identifier: "10150146071966729"     // some valid Facebook Photo fbid
        }

        Image {
            id: img
            anchors.top: parent.top
            anchors.bottom: parent.verticalCenter
            anchors.left: parent.left
            anchors.right: parent.right
            source: fbph.source
            onStatusChanged: {
                if (status == Image.Ready) {
                    // could upload a comment with:
                    fbph.uploadComment("I really like this photo!")
                    // could like the photo with:
                    fbph.like()
                    // or unlike it with:
                    fbph.unlike()
                }
            }
        }

        Component.onCompleted: {
            fb.populate()
        }
    }
    \endqml

    Note: the Facebook adapter currently only supports the
    \c ContentItemTypeFilter filter type, and does not support
    any form of sorting.
*/

FacebookInterface::FacebookInterface(QObject *parent)
    : SocialNetworkInterface(*(new FacebookInterfacePrivate(this)), parent)
{
}

FacebookInterface::~FacebookInterface()
{
}


/*!
    \qmlproperty QString Facebook::accessToken
    The access token to use when accessing the Facebook OpenGraph API.
*/
QString FacebookInterface::accessToken() const
{
    Q_D(const FacebookInterface);
    return d->accessToken;
}

void FacebookInterface::setAccessToken(const QString &token)
{
    Q_D(FacebookInterface);
    if (d->accessToken != token) {
        d->accessToken = token;
        emit accessTokenChanged();
    }
}

/*! \reimp */
void FacebookInterface::componentComplete()
{
    Q_D(FacebookInterface);
    // must set d->initialized to true.
    d->initialized = true;

    // now that we're initialized, perform any pending operations.
    if (d->populatePending) {
        populate();
    } else if (d->populateDataForUnseenPending) {
        populateDataForNode(d->pendingCurrentNodeIdentifier);
    }
}

/*! \reimp */
void FacebookInterface::populate()
{
    Q_D(FacebookInterface);
    // if no central node identifier is set by the client,
    // we load the "me" node by default.

    if (!d->initialized) {
        d->populatePending = true;
        return;
    }

    if (!node()) {
        if (nodeIdentifier().isEmpty()) {
            setNodeIdentifier(QLatin1String("me"));
        } else {
            populateDataForNode(nodeIdentifier());
        }
    } else {
        populateDataForNode(node());
    }
}

/*! \reimp */
QNetworkReply *FacebookInterface::getRequest(const QString &objectIdentifier, const QString &extraPath, const QStringList &whichFields, const QVariantMap &extraData)
{
    Q_D(FacebookInterface);
    if (!d->initialized) {
        qWarning() << Q_FUNC_INFO << "cannot complete get request: not initialized";
        return 0;
    }

    QVariantMap modifiedExtraData = extraData;
    if (!extraData.contains(QLatin1String("metadata")))
        modifiedExtraData.insert(QLatin1String("metadata"), QLatin1String("1")); // request "type" field.
    return d->qnam->get(QNetworkRequest(d->requestUrl(objectIdentifier, extraPath, whichFields, extraData)));
}

/*! \reimp */
QNetworkReply *FacebookInterface::postRequest(const QString &objectIdentifier, const QString &extraPath, const QVariantMap &data, const QVariantMap &extraData)
{
    Q_D(FacebookInterface);
    if (!d->initialized) {
        qWarning() << Q_FUNC_INFO << "cannot complete post request: not initialized";
        return 0;
    }

    // image upload is handled specially.
    if (extraData.value("isImageUpload").toBool())
        return d->uploadImage(objectIdentifier, extraPath, data, extraData);
    
    // create post data
    QString multipartBoundary = QLatin1String("-------Sska2129ifcalksmqq3");
    QByteArray postData;
    foreach (const QString &key, data.keys()) {
        postData.append("--"+multipartBoundary+"\r\n");
        postData.append("Content-Disposition: form-data; name=\"");
        postData.append(key);
        postData.append("\"\r\n\r\n");
        postData.append(data.value(key).toString());
        postData.append("\r\n");
    }
    postData.append("--"+multipartBoundary+"\r\n");

    // create request
    QNetworkRequest request(d->requestUrl(objectIdentifier, extraPath, QStringList(), extraData));
    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    request.setRawHeader("Accept-Language", "en-us,en;q=0.5");
    request.setRawHeader("Accept-Encoding", "gzip,deflate");
    request.setRawHeader("Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7");
    request.setRawHeader("Keep-Alive", "300");
    request.setRawHeader("Connection", "keep-alive");
    request.setRawHeader("Content-Type",QString("multipart/form-data; boundary="+multipartBoundary).toAscii());
    request.setHeader(QNetworkRequest::ContentLengthHeader, postData.size());

    // perform POST request
    return d->qnam->post(request, postData);
}

/*! \reimp */
QNetworkReply *FacebookInterface::deleteRequest(const QString &objectIdentifier, const QString &extraPath, const QVariantMap &extraData)
{
    Q_D(FacebookInterface);
    if (!d->initialized) {
        qWarning() << Q_FUNC_INFO << "cannot complete delete request: not initialized";
        return 0;
    }

    return d->qnam->deleteResource(QNetworkRequest(d->requestUrl(objectIdentifier, extraPath, QStringList(), extraData)));
}

/*! \reimp */
void FacebookInterface::updateInternalData(QList<CacheEntry*> data)
{
    Q_D(FacebookInterface);
    qWarning() << Q_FUNC_INFO << "filtering/sorting not implemented.  TODO!";

    // XXX TODO: filter the data in a better manner than linear searches...
    QList<CacheEntry*> filteredData = data;
    for (int i = 0; i < d->filters.size(); ++i) {
        QList<CacheEntry*> temp = filteredData;
        filteredData.clear();
        FilterInterface *currFilter = d->filters.at(i);
        for (int j = 0; j < data.size(); ++j) {
            CacheEntry *currEntry = data.at(j);
            if ((!currEntry->item && currFilter->matches(currEntry->data))
                    || (currEntry->item && currFilter->matches(currEntry->item))) {
                filteredData.append(currEntry);
            }
        }
    }

    // XXX TODO: sort the filtered data
    QList<CacheEntry*> sortedData = filteredData;

    // clear the internal data
    QModelIndex parent;
    if (d->internalData.count()) {
        beginRemoveRows(parent, 0, d->internalData.count());
        d->internalData = QList<CacheEntry*>();
        endRemoveRows();
    }

    // update the internal data
    beginInsertRows(parent, 0, sortedData.count());
    d->internalData = sortedData;
    endInsertRows();
    emit countChanged();
}

/*! \internal */
void FacebookInterface::retrieveRelatedContent(IdentifiableContentItemInterface *whichNode)
{
    Q_D(FacebookInterface);
    if (!whichNode) {
        qWarning() << Q_FUNC_INFO << "Cannot retrieve related content for null node!";
        return;
    }

    // XXX NOTE: Hard-coded Connections Ontology!
    // Here the ontology of connections for each type we are interested in, is hardcoded.
    // This should, perhaps, be updated to load them dynamically, or something...
    // But for now, this is good enough.

    QList<int> connectionTypes;
    QList<QStringList> connectionWhichFields;
    QList<int> connectionLimits;

    QList<FilterInterface*> allFilters = d->filters;
    if (allFilters.isEmpty()) {
        // if none are specified, fetch all possible connections for the node type.
        int nodeType = whichNode->type();
        switch (nodeType) {

            case FacebookInterface::Comment: {
                connectionTypes << FacebookInterface::Like;
                connectionWhichFields << QStringList();
                connectionLimits << -1;
            }
            break;

            case FacebookInterface::Photo: {
                connectionTypes << FacebookInterface::Like
                                << FacebookInterface::Comment
                                << FacebookInterface::Picture
                                << FacebookInterface::Tag;
                connectionWhichFields << QStringList()
                                      << QStringList()
                                      << QStringList()
                                      << QStringList();
                connectionLimits << -1 << -1 << -1 << -1;
            }
            break;

            case FacebookInterface::Album: {
                connectionTypes << FacebookInterface::Like
                                << FacebookInterface::Comment
                                << FacebookInterface::Picture
                                << FacebookInterface::Photo;
                connectionWhichFields << QStringList()
                                      << QStringList()
                                      << QStringList()
                                      << QStringList();
                connectionLimits << -1 << -1 << -1 << -1;
            }
            break;

            case FacebookInterface::User: {
                connectionTypes << FacebookInterface::Like
                                << FacebookInterface::Picture
                                << FacebookInterface::Photo
                                << FacebookInterface::Album
                                << FacebookInterface::Notification
                                << FacebookInterface::User; // friends... TODO: friends vs subscribers vs...
                connectionWhichFields << QStringList()
                                      << QStringList()
                                      << QStringList()
                                      << QStringList()
                                      << QStringList()
                                      << QStringList();
                connectionLimits << -1 << -1 << -1 << -1 << -1 << -1;
            }
            break;

            default: break;
        }
    } else {
        // otherwise, just fetch the ones specified by the filters.
        foreach (FilterInterface *currFilter, allFilters) {
            // XXX TODO: support "created since / updated since" filter (for Notifications at least)
            ContentItemTypeFilterInterface *citf = qobject_cast<ContentItemTypeFilterInterface*>(currFilter);
            if (!citf) {
                qWarning() << Q_FUNC_INFO << "Unsupported filter specified - the Facebook adapter only supports ContentItemType filters!";
                continue;
            }

            if (!connectionTypes.contains(citf->type())) {
                connectionTypes.append(citf->type());
                connectionWhichFields.append(citf->whichFields());
                connectionLimits.append(citf->limit());
            }
        }
    }

    // generate appropriate query string, using the Field Expansion query syntax of the Facebook OpenGraph API
    // eg: with currentNode = Photo; connectionTypes == comments,likes,tags; whichFields = id,name; limit = 10:
    // GET https://graph.facebook.com/<photo_id>/fields=comments.limit(10).fields(id,name),likes.limit(10).fields(id,name),tags.fields(id,name).limit(10)

    // XXX TODO: in the future, each of the Facebook-specific IdentifiableContentItemType classes should
    // provide private helper functions for the FacebookInterface to build the appropriate query.
    // e.g: QString FacebookAlbumInterface::relatedDataQuery(types, limits, whichfields);
    // That way we can provide "special case" code for every type in a neat, modular fashion.
    if (connectionTypes.size() == 1 && connectionTypes.at(0) == FacebookInterface::Photo
            && connectionLimits.at(0) == -1 && whichNode->type() == FacebookInterface::Album) {
        // special case code for FacebookAlbum "populate all photos" request
        QVariantMap extraData;
        extraData.insert(QLatin1String("limit"), QLatin1String("25"));
        d->currentReply = getRequest(whichNode->identifier(), FACEBOOK_ONTOLOGY_CONNECTIONS_PHOTOS, QStringList(), extraData);
        connect(d->currentReply, SIGNAL(finished()), this, SLOT(finishedHandler()));
        connect(d->currentReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(errorHandler(QNetworkReply::NetworkError)));
        connect(d->currentReply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrorsHandler(QList<QSslError>)));
    } else if (connectionTypes.size() == 1 && connectionTypes.at(0) == FacebookInterface::Notification
            && whichNode->type() == FacebookInterface::User) {
        // special case code for FacebookUser "populate notifications" request
        QVariantMap extraData;
        int specialLimit = 25;
        if (connectionLimits.at(0) == -1) {
            extraData.insert(QLatin1String("limit"), QLatin1String("25")); // get only 25 by default
        } else {
            specialLimit = connectionLimits.at(0);
            extraData.insert(QLatin1String("limit"), QString::number(connectionLimits.at(0)));
        }
        extraData.insert(QLatin1String("include_read"), QLatin1String("true"));
        d->currentReply = getRequest(whichNode->identifier(), FACEBOOK_ONTOLOGY_CONNECTIONS_NOTIFICATIONS, QStringList(), extraData);
        d->currentReply->setProperty("specialLimit", specialLimit); // we have to handle limit specially for notifications :-/
        connect(d->currentReply, SIGNAL(finished()), this, SLOT(finishedHandler()));
        connect(d->currentReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(errorHandler(QNetworkReply::NetworkError)));
        connect(d->currentReply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrorsHandler(QList<QSslError>)));
    } else {
        // generic query
        QString totalFieldsQuery;
        for (int i = 0; i < connectionTypes.size(); ++i) {
            bool addExtra = false;
            bool append = false;
            FacebookInterface::ContentItemType cit = static_cast<FacebookInterface::ContentItemType>(connectionTypes.at(i));
            switch (cit) {
                case FacebookInterface::NotInitialized:  qWarning() << Q_FUNC_INFO << "Invalid content item type specified in filter: NotInitialized";  break;
                case FacebookInterface::Unknown:         qWarning() << Q_FUNC_INFO << "Invalid content item type specified in filter: NotInitialized";  break;
                case FacebookInterface::ObjectReference: qWarning() << Q_FUNC_INFO << "Invalid content item type specified in filter: ObjectReference"; break;
                case FacebookInterface::Like:     addExtra = true;  append = true; totalFieldsQuery.append(FACEBOOK_ONTOLOGY_CONNECTIONS_LIKES);    break;
                case FacebookInterface::Tag:      addExtra = true;  append = true; totalFieldsQuery.append(FACEBOOK_ONTOLOGY_CONNECTIONS_TAGS);     break;
                case FacebookInterface::Picture:  addExtra = false; append = true; totalFieldsQuery.append(FACEBOOK_ONTOLOGY_CONNECTIONS_PICTURE);  break;
                case FacebookInterface::Location: addExtra = true; append = false; totalFieldsQuery.append(FACEBOOK_ONTOLOGY_CONNECTIONS_LOCATIONS); break; // not supported?
                case FacebookInterface::Comment:  addExtra = true; append = true; totalFieldsQuery.append(FACEBOOK_ONTOLOGY_CONNECTIONS_COMMENTS); break;
                case FacebookInterface::User:     addExtra = true; append = true; totalFieldsQuery.append(FACEBOOK_ONTOLOGY_CONNECTIONS_FRIENDS);  break; // subscriptions etc?
                case FacebookInterface::Album:    addExtra = true; append = true; totalFieldsQuery.append(FACEBOOK_ONTOLOGY_CONNECTIONS_ALBUMS);   break;
                case FacebookInterface::Photo:    addExtra = true; append = true; totalFieldsQuery.append(FACEBOOK_ONTOLOGY_CONNECTIONS_PHOTOS);   break;
                case FacebookInterface::Event:    addExtra = true; append = true; totalFieldsQuery.append(FACEBOOK_ONTOLOGY_CONNECTIONS_EVENTS);   break;
                case FacebookInterface::Notification: addExtra = true; append = true; totalFieldsQuery.append(FACEBOOK_ONTOLOGY_CONNECTIONS_NOTIFICATIONS);   break;
                default: break;
            }

            QString whichFieldsString;
            QStringList whichFields = connectionWhichFields.at(i);
            if (!whichFields.isEmpty())
                whichFieldsString = QLatin1String(".fields(") + whichFields.join(",") + QLatin1String(")");

            QString limitString;
            int limit = connectionLimits.at(i);
            if (limit == -1)
                limitString = QLatin1String(".limit(0)");
            else if (limit > 0)
                limitString = QLatin1String(".limit(") + QString::number(limit) + QLatin1String(")");

            if (append && addExtra && !limitString.isEmpty())
                totalFieldsQuery.append(limitString);
            if (append && addExtra && !whichFieldsString.isEmpty())
                totalFieldsQuery.append(whichFieldsString);
            if (append)
                totalFieldsQuery.append(QLatin1String(","));
        }

        totalFieldsQuery.chop(1); // remove trailing comma.
        QVariantMap extraData;
        extraData.insert("fields", totalFieldsQuery);

        // now start the request.
        d->currentReply = getRequest(whichNode->identifier(), QString(), QStringList(), extraData);
        connect(d->currentReply, SIGNAL(finished()), this, SLOT(finishedHandler()));
        connect(d->currentReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(errorHandler(QNetworkReply::NetworkError)));
        connect(d->currentReply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrorsHandler(QList<QSslError>)));
    }
}

/*! \reimp */
void FacebookInterface::populateDataForNode(IdentifiableContentItemInterface *currentNode)
{
    Q_D(FacebookInterface);
    if (currentNode == d->placeHolderNode) {
        // actually populating an unseen / pending node.
        return; // should have been queued as pending anyway.
    }

    d->status = SocialNetworkInterface::Busy;
    d->internalStatus = FacebookInterfacePrivate::PopulatingSeenNode;
    emit statusChanged();

    // clear the internal data
    if (d->internalData.count()) {
        QModelIndex parent;
        beginRemoveRows(parent, 0, d->internalData.count());
        d->internalData = QList<CacheEntry*>();
        endRemoveRows();
        emit countChanged();
    }

    // retrieve the related content
    retrieveRelatedContent(currentNode);

    // continued in continuePopulateDataForSeenNode().
}

#define FACEBOOK_CREATE_UNCACHED_ENTRY_FROM_DATA(data, type)                                \
    do {                                                                                    \
        QVariantList itemsList = data.value(FACEBOOK_ONTOLOGY_CONNECTIONS_DATA).toList();   \
        foreach (const QVariant &currVar, itemsList) {                                      \
            QVariantMap currMap = currVar.toMap();                                          \
            currMap.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE, type);                    \
            currMap.insert(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID,                             \
                    currMap.value(FACEBOOK_ONTOLOGY_METADATA_ID).toString());               \
            relatedContent.append(d->createUncachedEntry(currMap));                         \
        }                                                                                   \
    } while (0)

/*! \internal */
void FacebookInterface::continuePopulateDataForSeenNode(const QVariantMap &relatedData, const QUrl &requestUrl)
{
    Q_D(FacebookInterface);
    // We receive the related data and transform it into ContentItems.
    // Finally, we populate the cache for the node and update the internal model data.

    int currentCount = 0;
    QString continuationRequestUri;
    QList<CacheEntry *> relatedContent;
    if (d->continuationRequestActive) {
        // we are continuing a request, and thus don't overwrite the existing
        // cache entries, but instead append to them.
        bool ok = true;
        relatedContent = d->cachedContent(d->currentNode(), &ok);
        currentCount = relatedContent.size();
        if (!ok) {
            qWarning() << Q_FUNC_INFO << "Clobbering cached content in continuation request for node:" << d->currentNode()->identifier();
        }
    }

    // construct related content items from the request results.
    QStringList keys = relatedData.keys();
    foreach (const QString &key, keys) {
#if 0
qWarning() << "        " << key << " = " << FACEBOOK_DEBUG_VALUE_STRING_FROM_DATA(key, relatedData);
#endif
        if (d->outOfBandConnectionsLimit != -1 && currentCount >= d->outOfBandConnectionsLimit) {
            // we've already obtained enough data.
            break;
        }

        if (key == FACEBOOK_ONTOLOGY_CONNECTIONS_DATA) {
            // contains a list of objects, whose type should be described by the request uri
            QString reqPath = requestUrl.path();
            if (reqPath.endsWith(FACEBOOK_ONTOLOGY_CONNECTIONS_LIKES)) {
                FACEBOOK_CREATE_UNCACHED_ENTRY_FROM_DATA(relatedData, FacebookInterface::Like);
            } else if (reqPath.endsWith(FACEBOOK_ONTOLOGY_CONNECTIONS_COMMENTS)) {
                FACEBOOK_CREATE_UNCACHED_ENTRY_FROM_DATA(relatedData, FacebookInterface::Comment);
            } else if (reqPath.endsWith(FACEBOOK_ONTOLOGY_CONNECTIONS_TAGS)) {
                FACEBOOK_CREATE_UNCACHED_ENTRY_FROM_DATA(relatedData, FacebookInterface::Tag);
            } else if (reqPath.endsWith(FACEBOOK_ONTOLOGY_CONNECTIONS_PHOTOS)) {
                FACEBOOK_CREATE_UNCACHED_ENTRY_FROM_DATA(relatedData, FacebookInterface::Photo);
            } else if (reqPath.endsWith(FACEBOOK_ONTOLOGY_CONNECTIONS_ALBUMS)) {
                FACEBOOK_CREATE_UNCACHED_ENTRY_FROM_DATA(relatedData, FacebookInterface::Album);
            } else if (reqPath.endsWith(FACEBOOK_ONTOLOGY_CONNECTIONS_FRIENDS)) {
                FACEBOOK_CREATE_UNCACHED_ENTRY_FROM_DATA(relatedData, FacebookInterface::User);
            } else if (reqPath.endsWith(FACEBOOK_ONTOLOGY_CONNECTIONS_NOTIFICATIONS)) {
                FACEBOOK_CREATE_UNCACHED_ENTRY_FROM_DATA(relatedData, FacebookInterface::Notification);
            } else {
                qWarning() << Q_FUNC_INFO << "Informative: Unsupported data retrieved via edge:" << reqPath;
            }
        } else if (key == FACEBOOK_ONTOLOGY_CONNECTIONS_LIKES) {
            QVariantMap likesObject = relatedData.value(key).toMap();
            FACEBOOK_CREATE_UNCACHED_ENTRY_FROM_DATA(likesObject, FacebookInterface::Like);
        } else if (key == FACEBOOK_ONTOLOGY_CONNECTIONS_COMMENTS) {
            QVariantMap commentsObject = relatedData.value(key).toMap();
            FACEBOOK_CREATE_UNCACHED_ENTRY_FROM_DATA(commentsObject, FacebookInterface::Comment);
        } else if (key == FACEBOOK_ONTOLOGY_CONNECTIONS_TAGS) {
            QVariantMap tagsObject = relatedData.value(key).toMap();
            FACEBOOK_CREATE_UNCACHED_ENTRY_FROM_DATA(tagsObject, FacebookInterface::Tag);
        } else if (key == FACEBOOK_ONTOLOGY_CONNECTIONS_PHOTOS) {
            QVariantMap photosObject = relatedData.value(key).toMap();
            FACEBOOK_CREATE_UNCACHED_ENTRY_FROM_DATA(photosObject, FacebookInterface::Photo);
        } else if (key == FACEBOOK_ONTOLOGY_CONNECTIONS_ALBUMS) {
            QVariantMap albumsObject = relatedData.value(key).toMap();
            QVariantList albumsData = albumsObject.value(FACEBOOK_ONTOLOGY_CONNECTIONS_DATA).toList();
            FACEBOOK_CREATE_UNCACHED_ENTRY_FROM_DATA(albumsObject, FacebookInterface::Album);
        } else if (key == FACEBOOK_ONTOLOGY_CONNECTIONS_FRIENDS) {
            QVariantMap friendsObject = relatedData.value(key).toMap();
            FACEBOOK_CREATE_UNCACHED_ENTRY_FROM_DATA(friendsObject, FacebookInterface::User);
        } else if (key == FACEBOOK_ONTOLOGY_METADATA_PAGING) {
            QVariantMap pagingObject = relatedData.value(key).toMap();
            continuationRequestUri = pagingObject.value(FACEBOOK_ONTOLOGY_METADATA_PAGING_NEXT).toString();
        } else if (key == FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER
                || key == FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTPICTURE) {
            // can ignore this data - it's for the current node, which we already know.
        } else if (key == FACEBOOK_ONTOLOGY_CONNECTIONS_SUMMARY) {
            // can ignore this data - it just summarises how many unread notifications we have.
        } else {
            qWarning() << Q_FUNC_INFO << "Informative: Unsupported data retrieved:" << key;
        }
    }

    // We don't need to sort it here, as sorting is done in-memory during updateInternalData().
    // XXX TODO: make the entire filter/sort codepath more efficient, by guaranteeing that whatever
    // comes out of the cache must be sorted/filtered already?  Requires invalidating the entire
    // cache on filter/sorter change, however... hrm...

    bool ok = false;
    d->populateCache(d->currentNode(), relatedContent, &ok);
    if (!ok) {
        qWarning() << Q_FUNC_INFO << "Error: Unable to populate the cache for the current node:" << d->currentNode()->identifier();
    }

    // Update the model data.
    updateInternalData(relatedContent);

    // If we need to request more (paged) data, do so.
    if (continuationRequestUri.isEmpty()) {
        // there are no more results / result pages to retrieve.
        d->continuationRequestActive = false;
        d->status = SocialNetworkInterface::Idle;
        emit statusChanged();
    } else {
        // there are more results to retrieve.  Start a continuation request.
        d->continuationRequestActive = true;
        // grab the relevant parts of the continuation uri to create a new request.
        QUrl continuationUrl(continuationRequestUri);
        if (continuationUrl.queryItemValue(QLatin1String("access_token")).isEmpty())
            continuationUrl.addQueryItem(QLatin1String("access_token"), d->accessToken);
        d->currentReply = d->qnam->get(QNetworkRequest(continuationUrl));
        if (d->outOfBandConnectionsLimit != -1) {
            d->currentReply->setProperty("specialLimit", d->outOfBandConnectionsLimit);
        }
        connect(d->currentReply, SIGNAL(finished()), this, SLOT(finishedHandler()));
        connect(d->currentReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(errorHandler(QNetworkReply::NetworkError)));
        connect(d->currentReply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrorsHandler(QList<QSslError>)));
    }
}

/*! \reimp */
void FacebookInterface::populateDataForNode(const QString &unseenNodeIdentifier)
{
    Q_D(FacebookInterface);
    // This function should be implemented so that:
    // 0) the current model data should be set to empty
    // 1) the given node is requested from the service, with the given fields loaded
    // 2) when received, the node should be pushed to the nodeStack via d->pushNode(n)
    // 3) the related content data should be requested from the service, according to the filters
    // 4) when received, the related content data should be used to populate the cache via d->populateCache()
    // 5) finally, updateInternalData() should be called, passing in the new cache data.

    if (unseenNodeIdentifier != d->pendingCurrentNodeIdentifier) {
        qWarning() << Q_FUNC_INFO << "populating data for unseen node which isn't the pending current node!";
        return; // this is an error in the implementation of SocialNetworkInterface.
    }

    if (!d->initialized) {
        // we should delay this until we are initialized
        d->populateDataForUnseenPending = true;
        return;
    }

    d->status = SocialNetworkInterface::Busy;
    d->internalStatus = FacebookInterfacePrivate::PopulatingUnseenNode;
    emit statusChanged();

    // clear the internal data
    if (d->internalData.count()) {
        QModelIndex parent;
        beginRemoveRows(parent, 0, d->internalData.count());
        d->internalData = QList<CacheEntry*>();
        endRemoveRows();
        emit countChanged();
    }

    // get the unseen node data.
    d->currentReply = getRequest(unseenNodeIdentifier, QString(), QStringList(), QVariantMap());
    connect(d->currentReply, SIGNAL(finished()), this, SLOT(finishedHandler()));
    connect(d->currentReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(errorHandler(QNetworkReply::NetworkError)));
    connect(d->currentReply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrorsHandler(QList<QSslError>)));

    // continued in continuePopulateDataForUnseenNode().
}

/*! \internal */
void FacebookInterface::continuePopulateDataForUnseenNode(const QVariantMap &nodeData)
{
    Q_D(FacebookInterface);
    // having retrieved the node data, we construct the node, push it, and request
    // the related data required according to the filters.
    ContentItemInterface *convertedNode = contentItemFromData(this, nodeData);
    IdentifiableContentItemInterface *newCurrentNode = qobject_cast<IdentifiableContentItemInterface*>(convertedNode);
    if (!newCurrentNode) {
        if (convertedNode)
            convertedNode->deleteLater();
        d->status = SocialNetworkInterface::Error;
        d->error = SocialNetworkInterface::DataUpdateError;
        d->errorMessage = QLatin1String("Error retrieving node with identifier: ") + d->pendingCurrentNodeIdentifier;
        emit statusChanged();
        emit errorChanged();
        emit errorMessageChanged();
        return;
    }

    // push the retrieved node to the nodeStack.
    d->pushNode(newCurrentNode);
    d->pendingCurrentNodeIdentifier = QString();
    emit nodeChanged();

    // now that we have retrieved the node, now retrieve the related content.
    d->internalStatus = FacebookInterfacePrivate::PopulatingSeenNode;
    retrieveRelatedContent(newCurrentNode);
}

/*! \internal */
QString FacebookInterface::currentUserIdentifier() const
{
    Q_D(const FacebookInterface);
    // returns the object identifier associated with the "me" node, if loaded.
    return d->currentUserIdentifier;
}

/*! \reimp */
ContentItemInterface *FacebookInterface::contentItemFromData(QObject *parent, const QVariantMap &data) const
{
    Q_D(const FacebookInterface);
    // Construct the appropriate FacebookWhateverInterface for the given data.
    FacebookInterface::ContentItemType detectedType = static_cast<FacebookInterface::ContentItemType>(d->detectTypeFromData(data));
    switch (detectedType) {
        case FacebookInterface::Like: {
            FacebookLikeInterface *retn = new FacebookLikeInterface(parent);
            retn->classBegin();
            retn->setSocialNetwork(const_cast<FacebookInterface*>(this));
            setContentItemData(retn, data);
            retn->componentComplete();
            return retn;
        }
        break;

        case FacebookInterface::Comment: {
            FacebookCommentInterface *retn = new FacebookCommentInterface(parent);
            retn->classBegin();
            retn->setSocialNetwork(const_cast<FacebookInterface*>(this));
            setContentItemData(retn, data);
            retn->componentComplete();
            return retn;
        }
        break;

        case FacebookInterface::Photo: {
            FacebookPhotoInterface *retn = new FacebookPhotoInterface(parent);
            retn->classBegin();
            retn->setSocialNetwork(const_cast<FacebookInterface*>(this));
            setContentItemData(retn, data);
            retn->componentComplete();
            return retn;
        }
        break;

        case FacebookInterface::Album: {
            FacebookAlbumInterface *retn = new FacebookAlbumInterface(parent);
            retn->classBegin();
            retn->setSocialNetwork(const_cast<FacebookInterface*>(this));
            setContentItemData(retn, data);
            retn->componentComplete();
            return retn;
        }
        break;

        case FacebookInterface::User: {
            FacebookUserInterface *retn = new FacebookUserInterface(parent);
            retn->classBegin();
            retn->setSocialNetwork(const_cast<FacebookInterface*>(this));
            setContentItemData(retn, data);
            retn->componentComplete();
            return retn;
        }
        break;

        case FacebookInterface::Notification: {
            FacebookNotificationInterface *retn = new FacebookNotificationInterface(parent);
            retn->classBegin();
            retn->setSocialNetwork(const_cast<FacebookInterface*>(this));
            setContentItemData(retn, data);
            retn->componentComplete();
            return retn;
        }
        break;

        case FacebookInterface::Unknown: {
            qWarning() << Q_FUNC_INFO << "Unable to detect the type of the content item";
            IdentifiableContentItemInterface *retn = new IdentifiableContentItemInterface(parent);
            retn->classBegin();
            retn->setSocialNetwork(const_cast<FacebookInterface*>(this));
            setContentItemData(retn, data);
            retn->componentComplete();
            return retn;
        }
        break;

        default: qWarning() << Q_FUNC_INFO << "unsupported type:" << detectedType; break;
    }

    return 0;
}

/*! \internal */
FacebookObjectReferenceInterface *FacebookInterface::objectReference(QObject *parent, int type, QString identifier, QString name)
{
    // constructs a FacebookObjectReference which will be exposed as a Q_PROPERTY
    // of some content item which requires it (eg, significantOther, etc).

    QVariantMap data;
    data.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTTYPE, type);
    data.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER, identifier);
    data.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME, name);

    FacebookObjectReferenceInterface *fori = new FacebookObjectReferenceInterface(parent);
    fori->classBegin();
    setContentItemData(fori, data);
    fori->componentComplete();
    return fori;
}

/*! \internal */
QVariantMap FacebookInterface::facebookContentItemData(ContentItemInterface *contentItem)
{
    return contentItemData(contentItem);
}

/*! \internal */
void FacebookInterface::setFacebookContentItemData(ContentItemInterface *contentItem, const QVariantMap &data)
{
    setContentItemData(contentItem, data);
}

#include "moc_facebookinterface.cpp"
