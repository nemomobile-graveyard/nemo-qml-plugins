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

#ifndef SOCIALNETWORKINTERFACE_P_H
#define SOCIALNETWORKINTERFACE_P_H

#include "socialnetworkinterface.h"

#include "filterinterface.h"
#include "sorterinterface.h"

#include <QtCore/QVariantMap>
#include <QtCore/QList>
#include <QtCore/QString>

#include <QtNetwork/QSslError>
#include <QtNetwork/QNetworkReply>

class QNetworkAccessManager;
class IdentifiableContentItemInterface;
class FilterInterface;
class SorterInterface;

class SocialNetworkInterfacePrivate;
class CacheEntry
{
public:
    CacheEntry(const QVariantMap &d, ContentItemInterface *i = 0);
    ~CacheEntry();

    QVariantMap data;
    ContentItemInterface *item;

private:
    int refcount;
    friend class SocialNetworkInterfacePrivate;
};

class ArbitraryRequestHandler : public QObject
{
    Q_OBJECT

public:
    ArbitraryRequestHandler(SocialNetworkInterface *parent);
    ~ArbitraryRequestHandler();

    SocialNetworkInterface *q;
    QNetworkReply *reply;
    QString errorMessage;
    bool isError;

    bool request(int requestType, const QString &requestUri, const QVariantMap &queryItems = QVariantMap(), const QString &postData = QString());

public Q_SLOTS:
    void finishedHandler();
    void errorHandler(QNetworkReply::NetworkError err);
    void sslErrorsHandler(const QList<QSslError> &sslErrors);
};

class SocialNetworkInterfacePrivate
{
public:
    SocialNetworkInterfacePrivate(SocialNetworkInterface *q);
    ~SocialNetworkInterfacePrivate();

    SocialNetworkInterface * const q_ptr;
    QNetworkAccessManager *qnam;
    IdentifiableContentItemInterface *placeHolderNode;

    bool initialized;
    bool repopulatingCurrentNode;

    QString errorMessage;
    SocialNetworkInterface::ErrorType error;
    SocialNetworkInterface::Status status;

    QVariantMap relevanceCriteria;

    // --------------------- model data

    QHash<int, QByteArray> headerData;
    QList<CacheEntry*> internalData;

    // --------------------- filters/sorters functions

    static void filters_append(QDeclarativeListProperty<FilterInterface> *list, FilterInterface *filter);
    static FilterInterface *filters_at(QDeclarativeListProperty<FilterInterface> *list, int index);
    static void filters_clear(QDeclarativeListProperty<FilterInterface> *list);
    static int filters_count(QDeclarativeListProperty<FilterInterface> *list);
    QList<FilterInterface*> filters;

    static void sorters_append(QDeclarativeListProperty<SorterInterface> *list, SorterInterface *sorter);
    static SorterInterface *sorters_at(QDeclarativeListProperty<SorterInterface> *list, int index);
    static void sorters_clear(QDeclarativeListProperty<SorterInterface> *list);
    static int sorters_count(QDeclarativeListProperty<SorterInterface> *list);
    QList<SorterInterface*> sorters;

    // --------------------- node navigation functions

    QString pendingCurrentNodeIdentifier;

    IdentifiableContentItemInterface *currentNode() const;
    QString currentNodeIdentifier() const;
    void pushPlaceHolderNode(); // empty node.
    void pushNode(IdentifiableContentItemInterface *n);
    void nextNode();
    void prevNode();

private:
    void purgeDoomedNode(IdentifiableContentItemInterface *n);
    void maybePurgeDoomedNodes(int count, int direction);
    int currentNodePosition;                                 // the position of the current node in the nodeStack.
    int nodeStackSize;                                       // the number of nodes in the nodeStack (navigation)
    QList<IdentifiableContentItemInterface*> nodeStack;      // navigation breadcrumbs.

public:
    // --------------------- cache functions
    IdentifiableContentItemInterface *findCachedNode(const QString &nodeIdentifier);
    QList<CacheEntry*> cachedContent(IdentifiableContentItemInterface *n, bool *ok) const;
    void populateCache(IdentifiableContentItemInterface *n, const QList<CacheEntry*> c, bool *ok);
    CacheEntry *createUncachedEntry(const QVariantMap &data);
    CacheEntry *findCacheEntry(const QVariantMap &data, bool create = true);
    CacheEntry *findCacheEntry(ContentItemInterface *item, bool create = true);

private:
    void addEntryToNodeContent(IdentifiableContentItemInterface *item, CacheEntry *entry);
    void removeEntryFromNodeContent(IdentifiableContentItemInterface *item, CacheEntry *entry); // if after deref, count == 0, removes from cache list and deletes.
    void updateCacheEntry(CacheEntry *entry, ContentItemInterface *item, const QVariantMap &data = QVariantMap()) const;
    void derefCacheEntry(CacheEntry *entry); // if after deref, count == 0, removes from cache list and deletes.

    QList<CacheEntry*> cache; // the actual cache
    QHash<ContentItemInterface*, CacheEntry*> cachedItems; // "index" of cache entries which have items constructed.
    QMultiHash<IdentifiableContentItemInterface*, CacheEntry*> nodeContent; // cache entries which are connections/related content for a given node

    ArbitraryRequestHandler *arbitraryRequestHandler;

    Q_DECLARE_PUBLIC(SocialNetworkInterface)
};

#endif // SOCIALNETWORKINTERFACE_P_H
