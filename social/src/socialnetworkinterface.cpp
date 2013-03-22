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

#include "socialnetworkinterface.h"
#include "socialnetworkinterface_p.h"

#include "contentiteminterface.h"
#include "contentiteminterface_p.h"
#include "identifiablecontentiteminterface.h"
#include "filterinterface_p.h"
#include "sorterinterface_p.h"

#include <QtCore/QByteArray>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <QtDebug>

/*
    CacheEntry:

    The data in the model is actually a list of cache entries.
    Each entry consists of the QVariantMap of data for the object,
    and a (lazily constructed) ContentItem ptr.

    Specific implementations of the SocialNetwork interface should
    use \c SocialNetworkInterfacePrivate::createUncachedEntry() to
    create cache entries for data associated with a particular node,
    within their implementation of the \c populateDataForNode()
    functions.

    Once they have constructed cache entries for the related data
    objects, the implementation should call
    \c SocialNetworkInterfacePrivate::populateCache().
*/
CacheEntry::CacheEntry(const QVariantMap &data, ContentItemInterface *item)
    : data(data), item(item), refcount(0)
{
}

CacheEntry::~CacheEntry()
{
    if (item)
        delete item;
}

ArbitraryRequestHandler::ArbitraryRequestHandler(SocialNetworkInterface *parent)
    : QObject(parent), q(parent), reply(0), isError(false)
{
}

ArbitraryRequestHandler::~ArbitraryRequestHandler()
{
    if (reply) {
        reply->deleteLater();
    }
}

bool ArbitraryRequestHandler::request(int requestType, const QString &requestUri,
                                      const QVariantMap &queryItems, const QString &postData)
{
    if (reply) {
        qWarning() << Q_FUNC_INFO
                   << "Warning: cannot start arbitrary request: another arbitrary request is in progress";
        return false;
    }

    QList<QPair<QString, QString> > formattedQueryItems;
    QStringList queryItemKeys = queryItems.keys();
    foreach (const QString &key, queryItemKeys)
        formattedQueryItems.append(qMakePair<QString, QString>(key,
                                                               queryItems.value(key).toString()));

    QUrl url(requestUri);
    url.setQueryItems(formattedQueryItems);

    QNetworkReply *reply = 0;
    switch (requestType) {
    case SocialNetworkInterface::Get:
        reply = q->d_func()->networkAccessManager->get(QNetworkRequest(url));
        break;
    case SocialNetworkInterface::Post:
        reply = q->d_func()->networkAccessManager->post(QNetworkRequest(url),
                                                        QByteArray::fromBase64(postData.toLatin1()));
        break;
    default:
        reply = q->d_func()->networkAccessManager->deleteResource(QNetworkRequest(url));
        break;
    }

    if (reply) {
        reply = reply;
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(errorHandler(QNetworkReply::NetworkError)));
        connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
                this, SLOT(sslErrorsHandler(QList<QSslError>)));
        connect(reply, SIGNAL(finished()), this, SLOT(finishedHandler()));
        return true;
    }

    qWarning() << Q_FUNC_INFO << "Warning: cannot start arbitrary request: null reply";
    return false;
}

void ArbitraryRequestHandler::finishedHandler()
{
    QByteArray replyData;
    if (reply) {
        replyData = reply->readAll();
        reply->deleteLater();
        reply = 0;
    }

    QVariantMap responseData;
    bool errorOccurred = isError;
    if (isError) {
        // note that errors to arbitrary requests don't cause the SocialNetwork
        // to transition to the Error state.  They are unrelated to the model.
        responseData.insert(QLatin1String("error"), errorMessage);
        errorMessage = QString();
        isError = false;
    } else {
        bool ok = false;
        QVariantMap parsed = ContentItemInterfacePrivate::parseReplyData(replyData, &ok);
        if (!ok) {
            responseData.insert(QLatin1String("response"), replyData);
        } else {
            responseData = parsed;
        }
    }

    emit q->arbitraryRequestResponseReceived(errorOccurred, responseData);
}

void ArbitraryRequestHandler::errorHandler(QNetworkReply::NetworkError networkError)
{
    // TODO: This huge switch should be better, with QMetaEnum for example.
    // It should also be exported, in order to be used elsewhere
    switch (networkError) {
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

    isError = true;
}

void ArbitraryRequestHandler::sslErrorsHandler(const QList<QSslError> &sslErrors)
{
    errorMessage = QLatin1String("SSL error: ");
    if (sslErrors.isEmpty()) {
        errorMessage += QLatin1String("unknown SSL error");
    } else {
        foreach (const QSslError &sslError, sslErrors)
            errorMessage += sslError.errorString() + QLatin1String("; ");
        errorMessage.chop(2);
    }

    isError = true;
}

SocialNetworkInterfacePrivate::SocialNetworkInterfacePrivate(SocialNetworkInterface *q)
    : q_ptr(q)
    , networkAccessManager(0)
    , placeHolderNode(0)
    , initialized(false)
    , repopulatingCurrentNode(false)
    , error(SocialNetworkInterface::NoError)
    , status(SocialNetworkInterface::Initializing)
    , currentNodePosition(-1)
    , nodeStackSize(5)
    , arbitraryRequestHandler(0)
{
}

SocialNetworkInterfacePrivate::~SocialNetworkInterfacePrivate()
{
    // remove all cache entries.
    QList<IdentifiableContentItemInterface*> cacheEntries = nodeContent.keys();
    foreach (IdentifiableContentItemInterface *nodePtr, cacheEntries)
        purgeDoomedNode(nodePtr);
}

void SocialNetworkInterfacePrivate::init()
{
    Q_Q(SocialNetworkInterface);
    networkAccessManager = new QNetworkAccessManager(q);

    headerData.insert(SocialNetworkInterface::ContentItemRole, "contentItem");
    headerData.insert(SocialNetworkInterface::ContentItemTypeRole, "contentItemType");
    headerData.insert(SocialNetworkInterface::ContentItemDataRole, "contentItemData" );
    headerData.insert(SocialNetworkInterface::ContentItemIdentifierRole, "contentItemIdentifier");
    q->setRoleNames(headerData);

    // Construct the placeholder node.  This node is used as a placeholder
    // when the client sets a specific nodeIdentifier until the node can
    // be retrieved from the social network.
    placeHolderNode = new IdentifiableContentItemInterface(q);
    placeHolderNode->classBegin();
    placeHolderNode->componentComplete();

}

/*! \internal */
void SocialNetworkInterfacePrivate::filters_append(QDeclarativeListProperty<FilterInterface> *list,
                                                   FilterInterface *filter)
{
    SocialNetworkInterface *socialNetwork = qobject_cast<SocialNetworkInterface *>(list->object);
    if (socialNetwork && filter) {
        if (!filter->parent()) {
            filter->setParent(socialNetwork);
            filter->d_func()->ownedBySocialNetworkInterface = true;
        }
        socialNetwork->d_func()->filters.append(filter);
    }
}

/*! \internal */
FilterInterface *SocialNetworkInterfacePrivate::filters_at(QDeclarativeListProperty<FilterInterface> *list, int index)
{
    SocialNetworkInterface *socialNetwork = qobject_cast<SocialNetworkInterface *>(list->object);
    if (socialNetwork && socialNetwork->d_func()->filters.count() > index && index >= 0)
        return socialNetwork->d_func()->filters.at(index);
    return 0;
}

/*! \internal */
void SocialNetworkInterfacePrivate::filters_clear(QDeclarativeListProperty<FilterInterface> *list)
{
    // TODO: This might be achieved without using the private attribute
    // but with the parenting system
    // filter->setParent(this) and if (filter->parent() == this)
    SocialNetworkInterface *socialNetwork = qobject_cast<SocialNetworkInterface *>(list->object);
    if (socialNetwork) {
        foreach (FilterInterface *cf, socialNetwork->d_func()->filters) {
            if (cf->d_func()->ownedBySocialNetworkInterface) {
                cf->deleteLater();
            }
        }
        socialNetwork->d_func()->filters.clear();
    }
}

/*! \internal */
int SocialNetworkInterfacePrivate::filters_count(QDeclarativeListProperty<FilterInterface> *list)
{
    SocialNetworkInterface *socialNetwork = qobject_cast<SocialNetworkInterface *>(list->object);
    if (socialNetwork)
        return socialNetwork->d_func()->filters.count();
    return 0;
}

/*! \internal */
void SocialNetworkInterfacePrivate::sorters_append(QDeclarativeListProperty<SorterInterface> *list,
                                                   SorterInterface *sorter)
{
    SocialNetworkInterface *socialNetwork = qobject_cast<SocialNetworkInterface *>(list->object);
    if (socialNetwork && sorter) {
        if (!sorter->parent()) {
            sorter->setParent(socialNetwork);
            sorter->d_func()->ownedBySocialNetworkInterface = true;
        }
        socialNetwork->d_func()->sorters.append(sorter);
    }
}

/*! \internal */
SorterInterface *SocialNetworkInterfacePrivate::sorters_at(QDeclarativeListProperty<SorterInterface> *list, int index)
{
    SocialNetworkInterface *socialNetwork = qobject_cast<SocialNetworkInterface *>(list->object);
    if (socialNetwork && socialNetwork->d_func()->sorters.count() > index && index >= 0)
        return socialNetwork->d_func()->sorters.at(index);
    return 0;
}

/*! \internal */
void SocialNetworkInterfacePrivate::sorters_clear(QDeclarativeListProperty<SorterInterface> *list)
{
    SocialNetworkInterface *socialNetwork = qobject_cast<SocialNetworkInterface *>(list->object);
    if (socialNetwork) {
        foreach (SorterInterface *cs, socialNetwork->d_func()->sorters) {
            if (cs->d_func()->ownedBySocialNetworkInterface) {
                cs->deleteLater();
            }
        }
        socialNetwork->d_func()->sorters.clear();
    }
}

/*! \internal */
int SocialNetworkInterfacePrivate::sorters_count(QDeclarativeListProperty<SorterInterface> *list)
{
    SocialNetworkInterface *socialNetwork = qobject_cast<SocialNetworkInterface *>(list->object);
    if (socialNetwork)
        return socialNetwork->d_func()->sorters.count();
    return 0;
}

/*
    Returns the current node which is the central content item.
    The model is populated with data related to the node.
    The current node is always owned by the SocialNetwork base class;
    it might be deleted at any time (e.g., if the cache entry gets purged).
*/
IdentifiableContentItemInterface *SocialNetworkInterfacePrivate::currentNode() const
{
    // caller does NOT take ownership.  We can (and will) delete it whenever we like.

    if (currentNodePosition >= 0 && currentNodePosition < nodeStack.size())
        return nodeStack.at(currentNodePosition);
    return 0;
}

/*! \internal */
QString SocialNetworkInterfacePrivate::currentNodeIdentifier() const
{
    if (currentNodePosition >= 0 && currentNodePosition < nodeStack.size())
        return nodeStack.at(currentNodePosition)->identifier();
    return QString();
}

/*! \internal */
void SocialNetworkInterfacePrivate::purgeDoomedNode(IdentifiableContentItemInterface *n)
{
    QList<CacheEntry*> cacheData = nodeContent.values(n);
    foreach (CacheEntry *currData, cacheData)
        removeEntryFromNodeContent(n, currData);
    CacheEntry *nodeCacheEntry = findCacheEntry(n, false);
    if (nodeCacheEntry)
        derefCacheEntry(nodeCacheEntry);
}

/*! \internal */
void SocialNetworkInterfacePrivate::maybePurgeDoomedNodes(int count, int direction)
{
    // Removes \a count nodes from the node stack.  The nodes
    // will be removed from the top (most recent) of the stack
    // if direction == 1, and from the bottom (least recent)
    // of the stack if direction == 0.
    // Also purges the associated cached content data, if they
    // don't also appear elsewhere in the stack.

    if (direction != 0 && direction != 1) {
        qWarning() << Q_FUNC_INFO << "Error: invalid direction specified!";
        return;
    }

    if (count > nodeStack.size()) {
        qWarning() << Q_FUNC_INFO << "Error: not that many nodes in the stack!";
        return;
    }

    // XXX TODO: this is a terrible algorithm, that iterates over the nodeStack way too many times.
    for (int i = count; i > 0; --i) {
        // remove the ToS (or BoS) node.
        IdentifiableContentItemInterface *doomedNode = direction > 0 ? nodeStack.takeLast()
                                                                     : nodeStack.takeFirst();
        if (direction == 0) {
            // the current node position needs to be reduced, as all nodes' positions shift down.
            currentNodePosition--;
        }

        // determine whether we need to purge the doomed node.
        if (!nodeStack.contains(doomedNode)) {
            // the node doesn't appear anywhere else in the navigation breadcrumb trail.
            // so we have to delete it and purge our cache of content items for the node.
            purgeDoomedNode(doomedNode);
        }
    }

}

/*! \internal */
void SocialNetworkInterfacePrivate::pushPlaceHolderNode()
{
    pushNode(placeHolderNode);
}

/*! \internal */
void SocialNetworkInterfacePrivate::pushNode(IdentifiableContentItemInterface *node)
{
    // the caller is responsible for emitting dataChanged() etc.

    if (node == 0) {
        qWarning() << Q_FUNC_INFO << "Attempted to push null node!";
        return;
    }

    if (currentNodePosition >= nodeStack.size()) {
        qWarning() << Q_FUNC_INFO << "Current node not on stack!";
        return;
    }

    IdentifiableContentItemInterface *currentNode = 0;
    if (currentNodePosition >= 0)
        currentNode = nodeStack.at(currentNodePosition);

    if (currentNode == node && currentNode != placeHolderNode)
        return; // nothing to do.

    // Check to see if we need to replace the placeholder or current node.
    if (currentNode == placeHolderNode || repopulatingCurrentNode) {
        // this will happen when the node data that the
        // derived type requested is received.
        repopulatingCurrentNode = false;
        if (currentNodePosition != (nodeStack.size() - 1)) {
            qWarning() << Q_FUNC_INFO << "Error: placeholder node not the ToS!";
        } else {
            nodeStack.removeLast();
            nodeStack.append(node);
        }
        return;
    }

    // Otherwise, we're pushing a new navigable node to the nodeStack.
    if (currentNodePosition != (nodeStack.size()-1)) {
        // current navigation position is not the top of stack
        // ie, they pushed a bunch of nodes, then they called
        // SNI::previousNode() one or more times, and now they
        // are pushing a node.  This node becomes the new top
        // of stack.  Purge any cache entries beyond the current
        // position if applicable.
        maybePurgeDoomedNodes(currentNodePosition+1, 1);
    } else if (nodeStack.size() == nodeStackSize) {
        // current node position is already top of stack, and
        // we've reached our max for cached navigation steps.
        maybePurgeDoomedNodes(1, 0); // purge the bottom (least recently used) node.
    }

    nodeStack.append(node);
    currentNodePosition = nodeStack.size() - 1; // end up pointing to ToS.
}

/*! \internal */
void SocialNetworkInterfacePrivate::nextNode()
{
    // the caller is responsible for emitting dataChanged() etc.

    if (currentNodePosition == -1 || nodeStack.size() == 0) {
        qWarning() << Q_FUNC_INFO << "No nodes in cache!";
        return;
    }

    if (currentNodePosition == (nodeStack.size() - 1)) {
        qWarning() << Q_FUNC_INFO << "Already at last node in cache!";
        return;
    }

    currentNodePosition ++;
}

/*! \internal */
void SocialNetworkInterfacePrivate::prevNode()
{
    // the caller is responsible for emitting dataChanged() etc.

    if (currentNodePosition == -1 || nodeStack.size() == 0) {
        qWarning() << Q_FUNC_INFO << "No nodes in cache!";
        return;
    }

    if (currentNodePosition == 0) {
        qWarning() << Q_FUNC_INFO << "Already at first node in cache!";
        return;
    }

    currentNodePosition --;
}

/*! \internal */
IdentifiableContentItemInterface *SocialNetworkInterfacePrivate::findCachedNode(const QString &nodeIdentifier)
{
    for (int i = 0; i < nodeStack.size(); ++i) {
        if (nodeStack.at(i)->identifier() == nodeIdentifier) {
            return nodeStack.at(i);
        }
    }

    return 0;
}

/*! \internal */
QList<CacheEntry*> SocialNetworkInterfacePrivate::cachedContent(IdentifiableContentItemInterface *node, bool *ok) const
{
    // Types derived from SocialNetworkInterface should call this to retrieve cached content for the node

    if (!nodeContent.contains(node)) {
        *ok = false;
        return QList<CacheEntry*>();
    }

    *ok = true;
    return nodeContent.values(node);
}

/*! \internal */
CacheEntry *SocialNetworkInterfacePrivate::findCacheEntry(const QVariantMap &data, bool create)
{
    // have to do a slow search.  avoid this if possible.
    foreach (CacheEntry *e, cache) {
        if (e->data == data)
            return e;
    }

    if (!create)
        return 0;

    // no such cache entry.  create it, but DON'T append it to the cache.
    // we append it to the cache (and take ownership of it) when they call
    // addEntryToNodeContent().
    CacheEntry *newEntry = new CacheEntry(data);
    return newEntry;
}

/*! \internal */
CacheEntry *SocialNetworkInterfacePrivate::findCacheEntry(ContentItemInterface *item, bool create)
{
    if (cachedItems.contains(item))
        return cachedItems.value(item);

    if (!create)
        return 0;

    // no such cache entry.  create it, but DON'T append it to the cache.
    // we append it to the cache (and take ownership of it) when they call
    // addEntryToNodeContent().
    CacheEntry *newEntry = new CacheEntry(item->data(), item);
    return newEntry;
}

/*! \internal */
void SocialNetworkInterfacePrivate::addEntryToNodeContent(IdentifiableContentItemInterface *item, CacheEntry *entry)
{
    if (!nodeContent.contains(item) && currentNode() != item) {
        qWarning() << Q_FUNC_INFO << "No such node:" << item;
        return;
    }

    if (nodeContent.find(item, entry) != nodeContent.end()) {
        qWarning() << Q_FUNC_INFO << "Entry:" << entry << "already cached as content for node:" << item;
        return;
    }

    entry->refcount++;
    if (entry->refcount == 1) {
        // new cache entry.
        cache.append(entry);
    }

    nodeContent.insert(item, entry);
}

/*! \internal */
void SocialNetworkInterfacePrivate::removeEntryFromNodeContent(IdentifiableContentItemInterface *item, CacheEntry *entry)
{
    if (entry == 0)
        return;

    int removeCount = nodeContent.remove(item, entry);
    if (removeCount == 0) {
        qWarning() << Q_FUNC_INFO << "Entry:" << entry << "is not cached as content for node:" << item;
        return;
    } else if (removeCount > 1) {
        qWarning() << Q_FUNC_INFO << "Entry:" << entry << "was cached" << removeCount << "times as content for node:" << item;
    }

    derefCacheEntry(entry);
}

/*! \internal */
void SocialNetworkInterfacePrivate::updateCacheEntry(CacheEntry *entry, ContentItemInterface *item, const QVariantMap &data) const
{
    if (item)
        entry->item = item;
    if (data != QVariantMap())
        entry->data = data;
}

/*! \internal */
void SocialNetworkInterfacePrivate::derefCacheEntry(CacheEntry *entry)
{
    if (entry->refcount == 0)
        qWarning() << Q_FUNC_INFO << "Entry:" << entry << "has not been referenced in the cache";

    entry->refcount--;
    if (entry->refcount <= 0) {
        cache.removeAll(entry);
        delete entry;
    }
}

/*
    This function should be called by specific implementations of the
    SocialNetwork interface, to create a cache entry for the current
    node whose data is the given \a data, as part of the implementation
    for the \c populateDataForNode() functions.

    After the cache entries for the current node have all been created,
    the implementation should then call
    \c SocialNetworkInterfacePrivate::populateCache().
*/
CacheEntry *SocialNetworkInterfacePrivate::createUncachedEntry(const QVariantMap &data)
{
    // this function should be called by SocialNetworkInterface derived-types when
    // they retrieve data from the service during populateDataForNode().
    // After creating an uncached entry for each related content data object they receive
    // from the service, they should call populateCache().
    CacheEntry *newEntry = new CacheEntry(data);
    return newEntry;
}

/*
    This function should be called by specific implementations of the
    SocialNetwork interface, to populate the cache for the current
    node \a n with the cache entries \a c, as part of the implementation
    for the \c populateDataForNode() functions.

    This function will set \a ok to true if the node \a n is the current
    node as expected, and the cache could be populated.
*/
void SocialNetworkInterfacePrivate::populateCache(IdentifiableContentItemInterface *node, const QList<CacheEntry*> cacheEntries, bool *ok)
{
    // Types derived from SocialNetworkInterface should call this to populate the cache
    // NOTE: we don't have any limits on cache size.  XXX TODO: something sensible?

    if (currentNode() != node) {
        // the populated node is not the current node... this is an error.
        qWarning() << Q_FUNC_INFO << "Attempted to populate cache for non-current node!";
        *ok = false;
        return;
    }

    *ok = true;

    QList<CacheEntry*> existingGoodEntries;
    QList<CacheEntry*> newCacheEntries;
    if (nodeContent.contains(node)) {
        // updating existing cache entry.
        QList<CacheEntry*> oldData = nodeContent.values(node);
        QList<CacheEntry*> doomedData;
        foreach (CacheEntry *currData, oldData) {
            if (cacheEntries.contains(currData)) {
                existingGoodEntries.append(currData);
            } else {
                doomedData.append(currData);
            }
        }

        // purge old entries from the cache
        foreach (CacheEntry *doomedContent, doomedData) {
            // not contained in the updated cache.
            removeEntryFromNodeContent(node, doomedContent);
        }

        // add new entries to the cache
        foreach (CacheEntry *newEntry, cacheEntries) {
            if (!existingGoodEntries.contains(newEntry)) {
                newCacheEntries.append(newEntry);
            }
        }
    } else {
        // new cache entry.
        newCacheEntries = cacheEntries;
    }

    // populate the cache for the node n from the content c.
    foreach (CacheEntry *currData, newCacheEntries) {
        addEntryToNodeContent(node, currData);
    }
}

//----------------------------------------------------

/*!
    \qmltype SocialNetwork
    \instantiates SocialNetworkInterface
    \inqmlmodule org.nemomobile.social 1
    \brief Provides an abstraction API for graph- or model-based social
    network APIs.

    The SocialNetwork type should never be used directly by clients.
    Instead, clients should use specific implementations of the SocialNetwork
    interface, such as the Facebook adapter.

    The SocialNetwork type provides a generic API which allows content
    to be retrieved from a social network and exposed via a model.
    The API consists of a central \c node which is an IdentifiableContentItem,
    which may be specified by the client via the \c nodeIdentifier property.
    The data in the model will be populated from the graph connections of
    the node.

    The model roles are as follows:
    \list
    \li contentItem - the instantiated ContentItem related to the node
    \li contentItemType - the type of the ContentItem related to the node
    \li contentItemData - the underlying QVariantMap data of the ContentItem related to the node
    \li contentItemIdentifier - the identifier of the ContentItem related to the node, or an empty string
    \endlist

    Please see the documentation of the Facebook adapter for an example
    of how clients can use the SocialNetwork model in an application.
*/  

SocialNetworkInterface::SocialNetworkInterface(QObject *parent)
    : QAbstractListModel(parent), d_ptr(new SocialNetworkInterfacePrivate(this))
{
    Q_D(SocialNetworkInterface);
    d->init();
}

SocialNetworkInterface::SocialNetworkInterface(SocialNetworkInterfacePrivate &dd, QObject *parent)
    : QAbstractListModel(parent), d_ptr(&dd)
{
    Q_D(SocialNetworkInterface);
    d->init();
}

SocialNetworkInterface::~SocialNetworkInterface()
{
}


void SocialNetworkInterface::classBegin()
{
    Q_D(SocialNetworkInterface);
    d->initialized = false;
}

void SocialNetworkInterface::componentComplete()
{
    Q_D(SocialNetworkInterface);
    // If you override this implementation, you MUST set d->initialized=true.
    d->initialized = true;
}

/*!
    \qmlmethod void SocialNetwork::nextNode()
    Navigates to the next node in the node stack, if it exists.
    The data in the model will be populated from the cache, if cached
    data for the node exists.  If you want to repopulate the data from
    the social network, you must call \c setNodeIdentifier() manually.

    The node stack is built up from successive changes to the
    \c nodeIdentifier property.
*/
void SocialNetworkInterface::nextNode()
{
    Q_D(SocialNetworkInterface);
    IdentifiableContentItemInterface *oldNode = d->currentNode();
    d->nextNode();
    IdentifiableContentItemInterface *newNode = d->currentNode();
    if (oldNode != newNode) {
        bool hasCachedContent = false;
        QList<CacheEntry*> data = d->cachedContent(newNode, &hasCachedContent);
        if (hasCachedContent) {
            // call derived class data update:
            //   perform filtering/sorting based on the defined stuff.
            //   and then emit dataChanged() etc signals.
            updateInternalData(data);
        } else {
            // call derived class data populate:
            //   d->populateCache() etc once it's finished retrieving.
            //   and then updateInternalData() itself.
            populateDataForNode(newNode);
        }
    }
}

/*!
    \qmlmethod void SocialNetwork::previousNode()
    Navigates to the previous node in the node stack, if it exists.
    The data in the model will be populated from the cache, if cached
    data for the node exists.  If you want to repopulate the data from
    the social network, you must call \c setNodeIdentifier() manually.

    The node stack is built up from successive changes to the
    \c nodeIdentifier property.
*/
void SocialNetworkInterface::previousNode()
{
    Q_D(SocialNetworkInterface);
    IdentifiableContentItemInterface *oldNode = d->currentNode();
    d->prevNode();
    IdentifiableContentItemInterface *newNode = d->currentNode();
    if (oldNode != newNode) {
        bool hasCachedContent = false;
        QList<CacheEntry*> data = d->cachedContent(newNode, &hasCachedContent);
        if (hasCachedContent) {
            // call derived class data update:
            //   perform filtering/sorting based on the defined stuff.
            //   and then emit dataChanged() etc signals.
            updateInternalData(data);
        } else {
            // call derived class data populate:
            //   d->populateCache() etc once it's finished retrieving.
            //   and then updateInternalData() itself.
            populateDataForNode(newNode);
        }
    }
}

/*!
    \qmlmethod QObject *SocialNetwork::relatedItem(int index)
    Returns the ContentItem which is related to the node from the given
    \a index of the model data.  This is identical to calling
    \c data() for the given model index and specifying the \c contentItem
    role.

    \note Although this function will always return a pointer to a
    ContentItem, the return type of the function is QObject*, so that
    this function can be used via QMetaObject::invokeMethod().
*/
QObject *SocialNetworkInterface::relatedItem(int index) const
{
    QVariant cv = data(QAbstractListModel::index(index), SocialNetworkInterface::ContentItemRole);
    if (!cv.isValid())
        return 0;
    ContentItemInterface *ci = cv.value<ContentItemInterface*>();
    
    return ci;
}

/*!
    \qmlproperty SocialNetwork::Status SocialNetwork::status
    Holds the current status of the social network.
*/
SocialNetworkInterface::Status SocialNetworkInterface::status() const
{
    Q_D(const SocialNetworkInterface);
    return d->status;
}

/*!
    \qmlproperty SocialNetwork::ErrorType SocialNetwork::error
    Holds the most recent error which occurred during initialization
    or a network request.  Note that it will not be reset if subsequent
    operations are successful.
*/
SocialNetworkInterface::ErrorType SocialNetworkInterface::error() const
{
    Q_D(const SocialNetworkInterface);
    return d->error;
}

/*!
    \qmlproperty QString SocialNetwork::errorMessage
    Holds the message associated with the most recent error which occurred
    during initialization or a network request.  Note that it will not be
    reset if subsequent operations are successful.
*/
QString SocialNetworkInterface::errorMessage() const
{
    Q_D(const SocialNetworkInterface);
    return d->errorMessage;
}

/*!
    \qmlproperty QString SocialNetwork::nodeIdentifier
    Holds the identifier of the "central" content item.  This content item
    is the \c node of the current view of the social network graph.
    The data in the model will be populated from the graph connections
    to the node.

    If this property is not set, the node will be initialized to the
    current user node by the specific social network implementation adapter.

    As the client changes the \c nodeIdentifier, the SocialNetwork will
    request the related data from the network, and build up a node stack
    of visited nodes.  For each visited node, a cache of related content
    items (model data) is stored.  The size of the cache is implementation
    specific.

    Clients can later navigate down and up the stack using the \l previousNode() and
    \l nextNode() functions respectively.  Those operations are very cheap
    as they do not trigger any network requests in the common case.

    If the \c nodeIdentifier is set to an identifier which isn't represented
    in the node stack, the \c node property will be set to an empty placeholder
    node until the network request completes and the node can be populated with
    the downloaded data.

    If the \c nodeIdentifier is set to the identifier of the current node,
    the cached data for the node will be cleared and the node and its related
    data will be reloaded from the network.
*/
QString SocialNetworkInterface::nodeIdentifier() const
{
    Q_D(const SocialNetworkInterface);
    if (d->pendingCurrentNodeIdentifier.isEmpty())
        return d->currentNodeIdentifier();  // normal case.
    return d->pendingCurrentNodeIdentifier; // status == Fetching, not sure if it's real yet.
}

void SocialNetworkInterface::setNodeIdentifier(const QString &contentItemIdentifier)
{
    Q_D(SocialNetworkInterface);
    IdentifiableContentItemInterface *cachedNode = d->findCachedNode(contentItemIdentifier);
    if (d->currentNode() && contentItemIdentifier == d->currentNode()->identifier()) {
        // resetting the current node.  This tells us to reload the node, clear its cache and repopulate.
        d->repopulatingCurrentNode = true;
        d->pendingCurrentNodeIdentifier = contentItemIdentifier;
        populateDataForNode(contentItemIdentifier); // "unseen node" without pushing placeholder.
    } else if (!cachedNode) {
        // Unseen node.
        // call derived class data populate:
        //   d->populateCache() etc once it's finished retrieving.
        //   d->pushNode(newNodePtr).
        //   and then updateInternalData() itself.
        d->pendingCurrentNodeIdentifier = contentItemIdentifier;
        d->pushPlaceHolderNode();
        emit nodeChanged();
        populateDataForNode(contentItemIdentifier); // XXX TODO: do we really want to trigger populate?  or wait for user to call populate?
    } else {
        // We've seen this node before and have it cached.
        bool hasCachedContent = false;
        QList<CacheEntry*> data = d->cachedContent(cachedNode, &hasCachedContent);
        if (hasCachedContent) {
            // call derived class data update:
            //   perform filtering/sorting based on the defined stuff.
            //   and then emit dataChanged() etc signals.
            d->pushNode(cachedNode);
            updateInternalData(data);
        } else {
            qWarning() << Q_FUNC_INFO << "Error: cached node has no cached content!";
        }
    }
}

/*!
    \qmlproperty IdentifiableContentItem *SocialNetwork::node
    Holds the "central" content item, or node, which defines the
    current view of the social network graph.  The data exposed in the
    SocialNetwork model will reflect the connections to the node.

    The node must be an identifiable content item (that is, it must
    have a unique identifier in the social network graph).
    Clients cannot set the node property directly, but instead must
    set the \c nodeIdentifier property.
*/
IdentifiableContentItemInterface *SocialNetworkInterface::node() const
{
    Q_D(const SocialNetworkInterface);
    return d->currentNode();
}

/*!
    \qmlproperty QVariantMap SocialNetwork::relevanceCriteria
    Holds the social-network-specific relevance criteria which will
    be used to calculate the relevance of related content items.
    This relevance can be important in filtering and sorting operations.
*/
QVariantMap SocialNetworkInterface::relevanceCriteria() const
{
    Q_D(const SocialNetworkInterface);
    return d->relevanceCriteria;
}

void SocialNetworkInterface::setRelevanceCriteria(const QVariantMap &relevanceCriteria)
{
    Q_D(SocialNetworkInterface);
    if (d->relevanceCriteria != relevanceCriteria) {
        d->relevanceCriteria = relevanceCriteria;
        emit relevanceCriteriaChanged();
    }
}

/*!
    \qmlproperty QDeclarativeListProperty<Filter> SocialNetwork::filters
    Holds the list of filters which will be applied to the related content
    of the node.  Only those related content items which match each of the
    filters will be exposed as data in the model.

    Specific implementations of the SocialNetwork interface may not support
    certain standard filter types, or they may not support filtering at all.
*/
QDeclarativeListProperty<FilterInterface> SocialNetworkInterface::filters()
{
    return QDeclarativeListProperty<FilterInterface>(this, 0,
            &SocialNetworkInterfacePrivate::filters_append,
            &SocialNetworkInterfacePrivate::filters_count,
            &SocialNetworkInterfacePrivate::filters_at,
            &SocialNetworkInterfacePrivate::filters_clear);
}

/*!
    \qmlproperty QDeclarativeListProperty<Sorter> SocialNetwork::sorters
    Holds the list of sorters which will be applied to the related content
    of the node.  The order of sorters in the list is important, as it
    defines which sorting is applied first.

    Specific implementations of the SocialNetwork interface may not support
    certain standard sorter types, or they may not support sorting at all.
*/
QDeclarativeListProperty<SorterInterface> SocialNetworkInterface::sorters()
{
    return QDeclarativeListProperty<SorterInterface>(this, 0,
            &SocialNetworkInterfacePrivate::sorters_append,
            &SocialNetworkInterfacePrivate::sorters_count,
            &SocialNetworkInterfacePrivate::sorters_at,
            &SocialNetworkInterfacePrivate::sorters_clear);
}

/*!
    \qmlproperty int SocialNetwork::count
    Returns the number of content items related to the \c node
    are exposed in the model.  Only those content items which
    match the specified \c filters will be exposed in the model,
    if the specific implementation of the SocialNetwork interface
    supports every filter in the \c filters list.
*/
int SocialNetworkInterface::count() const
{
    Q_D(const SocialNetworkInterface);
    return d->internalData.count();
}

int SocialNetworkInterface::rowCount(const QModelIndex &index) const
{
    Q_D(const SocialNetworkInterface);
    // only allow non-valid (default) parent index.
    if (index.isValid())
        return 0;
    return d->internalData.count();
}

int SocialNetworkInterface::columnCount(const QModelIndex &index) const
{
    // only allow non-valid (default) parent index.
    if (index.isValid())
        return 0;
    return 1;
}

QVariant SocialNetworkInterface::data(const QModelIndex &index, int role) const
{
    Q_D(const SocialNetworkInterface);
    if (!index.isValid() || index.row() >= d->internalData.count() || index.row() < 0)
        return QVariant();

    CacheEntry *cacheEntry = d->internalData.at(index.row());

    switch (role) {
        case ContentItemTypeRole: return QVariant::fromValue(cacheEntry->data.value(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMTYPE).toInt());
        case ContentItemDataRole: return QVariant::fromValue(cacheEntry->data);
        case ContentItemIdentifierRole: return QVariant::fromValue(cacheEntry->data.value(NEMOQMLPLUGINS_SOCIAL_CONTENTITEMID).toString());
        case ContentItemRole: {
            if (cacheEntry->item)
                return QVariant::fromValue(cacheEntry->item);
            // instantiate the item.
            ContentItemInterface *newItem = contentItemFromData(const_cast<SocialNetworkInterface*>(this), cacheEntry->data);
            d->updateCacheEntry(cacheEntry, newItem); // update the cache.
            return QVariant::fromValue(newItem);
        }
        break;
        default: return QVariant();
    }
}

QVariant SocialNetworkInterface::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_D(const SocialNetworkInterface);
    // Not a table model, so perhaps this is wrong.

    if (orientation != Qt::Horizontal)
        return QVariant();

    if (role == Qt::DisplayRole) {
        if (section < d->headerData.size()) {
            return d->headerData.value(section);
        }
    }

    return QVariant();
}

/*!
    \qmlmethod bool SocialNetwork::arbitraryRequest(SocialNetwork::RequestType requestType, const QString &requestUri, const QVariantMap &queryItems = QVariantMap(), const QString &postData = QString())

    Performs the HTTP request of the specified \a requestType (\c Get, \c Post or \c Delete) with
    the specified \a requestUri and \a queryItems.  If the request is a Post request, the given
    \a postData will be converted to a QByteArray via \c{QByteArray::fromBase64(postData.toLatin1())}
    and used as the \c Post data.

    When a successfully started request is completed, the \c arbitraryRequestResponseReceived()
    signal will be emitted, with the response data included as the \c data parameter.

    The request will not be started successfully if another arbitrary request is in progress.
    Returns true if the request could be started successfully, false otherwise. 
*/
bool SocialNetworkInterface::arbitraryRequest(int requestType, const QString &requestUri, const QVariantMap &queryItems, const QString &postData)
{
    Q_D(SocialNetworkInterface);
    if (!d->arbitraryRequestHandler)
        d->arbitraryRequestHandler = new ArbitraryRequestHandler(this);
    return d->arbitraryRequestHandler->request(requestType, requestUri, queryItems, postData);
}

QVariantMap SocialNetworkInterface::contentItemData(ContentItemInterface *contentItem) const
{
    // Helper function for SocialNetworkInterface-derived types
    return contentItem->dataPrivate();
}

void SocialNetworkInterface::setContentItemData(ContentItemInterface *contentItem, const QVariantMap &data) const
{
    // Helper function for SocialNetworkInterface-derived types
    contentItem->setDataPrivate(data);
}

/*
    Specific implementations of the SocialNetwork interface MUST implement this
    function.  It will be called to populate the model data as a filtered and
    sorted view of the content items related to the specified node in the
    social network.
*/
void SocialNetworkInterface::populate()
{
    qWarning() << Q_FUNC_INFO << "Error: this function MUST be implemented by derived types!";
}

/*
    Specific implementations of the SocialNetwork interface MUST implement this
    function.  It must be implemented such that it performs the appropriate
    get request to retrieve the data for the specified \c objectIdentifier, or
    data related to that object according to the given \c extraPath parameter.
    If possible, only the data specified by the \c whichFields parameter should
    be retrieved, to minimise network usage.  The \c extraData parameter is
    implementation specific, and may be used to modify the behaviour of the request.
*/
QNetworkReply *SocialNetworkInterface::getRequest(const QString &, const QString &, const QStringList &, const QVariantMap &)
{
    qWarning() << Q_FUNC_INFO << "Error: this function MUST be implemented by derived types!";
    return 0;
}

/*
    Specific implementations of the SocialNetwork interface MUST implement this
    function.  It must be implemented such that it performs the appropriate
    post request to upload the \c data for the specified \c objectIdentifier, or
    data related to that object according to the given \c extraPath parameter.
    The \c extraData parameter is implementation specific, and may be used to
    modify the behaviour of the request.
*/
QNetworkReply *SocialNetworkInterface::postRequest(const QString &, const QString &, const QVariantMap &, const QVariantMap &)
{
    qWarning() << Q_FUNC_INFO << "Error: this function MUST be implemented by derived types!";
    return 0;
}

/*
    Specific implementations of the SocialNetwork interface MUST implement this
    function.  It must be implemented such that it performs the appropriate
    delete request to delete the object identified by the specified
    \c objectIdentifier, or data related to that object according to the given
    \c extraPath parameter.  The \c extraData parameter is implementation specific,
    and may be used to modify the behaviour of the request.
*/
QNetworkReply *SocialNetworkInterface::deleteRequest(const QString &, const QString &, const QVariantMap &)
{
    qWarning() << Q_FUNC_INFO << "Error: this function MUST be implemented by derived types!";
    return 0;
}

/*
    Specific implementations of the SocialNetwork interface MUST implement this
    function.  It must return an instance of the correct ContentItem-derived type
    given the QVariantMap of data.  This function is called when the \c contentItem
    role for a specific model index is requested via the model data() function, to
    instantiate the content item from the content item data lazily.
*/
ContentItemInterface *SocialNetworkInterface::contentItemFromData(QObject *, const QVariantMap &) const
{
    qWarning() << Q_FUNC_INFO << "Error: this function MUST be implemented by derived types!";
    return 0;
}

/*
    Specific implementations of the SocialNetwork interface MUST implement this
    function.  It must be implemented so that:
    1) the provided data should have non-filters-matching-entries removed
    2) the filtered data should then be sorted according to the sorters
    3) the d->internalData list should be set
    4) finally, dataChanged() and any other model signals should be emitted
*/
void SocialNetworkInterface::updateInternalData(QList<CacheEntry*>)
{
    qWarning() << Q_FUNC_INFO << "Error: this function MUST be implemented by derived types!";
}

/*
    Specific implementations of the SocialNetwork interface MUST implement this
    function.  It must be implemented so that:
    0) the current model data should be set to empty
    1) the related content data should be requested from the service, according to the filters
    2) when received, the related content data should be used to populate the cache via d->populateCache()
    3) finally, updateInternalData() should be called, passing in the new cache data.
*/
void SocialNetworkInterface::populateDataForNode(IdentifiableContentItemInterface *)
{
    qWarning() << Q_FUNC_INFO << "Error: this function MUST be implemented by derived types!";
}

/*
    Specific implementations of the SocialNetwork interface MUST implement this
    function.  It must be implemented so that:
    0) the current model data should be set to empty
    1) the given node is requested from the service
    2) when received, the node should be pushed to the nodeStack via d->pushNode(n)
    3) the related content data should be requested from the service, according to the filters
    4) when received, the related content data should be used to populate the cache via d->populateCache()
    5) finally, updateInternalData() should be called, passing in the new cache data.
*/
void SocialNetworkInterface::populateDataForNode(const QString &)
{
    qWarning() << Q_FUNC_INFO << "Error: this function MUST be implemented by derived types!";
}

bool SocialNetworkInterface::isInitialized() const
{
    Q_D(const SocialNetworkInterface);
    // Helper function for ContentItemInterface
    return d->initialized;
}

#include "moc_socialnetworkinterface.cpp"

