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

#ifndef SOCIALNETWORKINTERFACE_H
#define SOCIALNETWORKINTERFACE_H

#include "filterinterface.h"
#include "sorterinterface.h"

#include <QtCore/QAbstractListModel>
#include <QtCore/QModelIndex>
#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include <QtCore/QStringList>
#include <QtCore/QString>
#include <QtDeclarative/QDeclarativeParserStatus>
#include <QtDeclarative/QDeclarativeListProperty>

class QNetworkReply;

class ContentItemInterface;
class IdentifiableContentItemInterface;

/*
 * NOTE: if you construct one of these in C++ directly,
 * you MUST call classBegin() and componentCompleted()
 * directly after construction.
 */

class CacheEntry;
class SocialNetworkInterfacePrivate;
class SocialNetworkInterface : public QAbstractListModel, public QDeclarativeParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QDeclarativeParserStatus)

    Q_PROPERTY(QString nodeIdentifier READ nodeIdentifier WRITE setNodeIdentifier NOTIFY nodeIdentifierChanged)
    Q_PROPERTY(IdentifiableContentItemInterface *node READ node NOTIFY nodeChanged)
    Q_PROPERTY(QVariantMap relevanceCriteria READ relevanceCriteria WRITE setRelevanceCriteria NOTIFY relevanceCriteriaChanged)
    Q_PROPERTY(QDeclarativeListProperty<FilterInterface> filters READ filters)
    Q_PROPERTY(QDeclarativeListProperty<SorterInterface> sorters READ sorters)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(ErrorType error READ error NOTIFY errorChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

    Q_ENUMS(Status)
    Q_ENUMS(ErrorType)
    Q_ENUMS(ContentType)
    Q_ENUMS(RequestType)

public:
    enum Status {
        Initializing,
        Idle,
        Busy,
        Error,
        Invalid
    };

    enum ErrorType {
        NoError = 0,
        AccountError = 1,
        SignOnError = 2,
        BusyError = 3,
        RequestError = 4,
        DataUpdateError = 5,
        OtherError = 6,
        LastError = 255
    };

    enum Roles {
        ContentItemRole = Qt::UserRole + 1,
        ContentItemTypeRole,
        ContentItemDataRole,
        ContentItemIdentifierRole // 0 for unidentifiable content items
    };

    enum ContentType {
        NotInitialized = 0,
        Unknown = 1
    };

    enum RequestType {
        Get = 0,
        Post,
        Delete
    };

public:
    explicit SocialNetworkInterface(QObject *parent = 0);
    virtual ~SocialNetworkInterface();

    // QDeclarativeParserStatus
    virtual void classBegin();
    virtual void componentComplete();

    // QAbstractListModel
    int rowCount(const QModelIndex &index = QModelIndex()) const;
    int columnCount(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    // invokable api.
    Q_INVOKABLE virtual void populate(); // count/startIndex?
    Q_INVOKABLE void nextNode();
    Q_INVOKABLE void previousNode();
    Q_INVOKABLE QObject *relatedItem(int index) const;

    // property accessors.
    Status status() const;
    ErrorType error() const;
    QString errorMessage() const;

    QString nodeIdentifier() const;
    IdentifiableContentItemInterface *node() const;
    QVariantMap relevanceCriteria() const;
    QDeclarativeListProperty<FilterInterface> filters();
    QDeclarativeListProperty<SorterInterface> sorters();
    int count() const;

    // property mutators.
    void setNodeIdentifier(const QString &contentItemIdentifier);
    void setRelevanceCriteria(const QVariantMap &rc);

Q_SIGNALS:
    void statusChanged();
    void errorChanged();
    void errorMessageChanged();

    void nodeIdentifierChanged();
    void nodeChanged();
    void relevanceCriteriaChanged();
    void countChanged();

public:
    Q_INVOKABLE bool arbitraryRequest(int requestType, const QString &requestUri, const QVariantMap &queryItems = QVariantMap(), const QString &postData = QString());
Q_SIGNALS:
    void arbitraryRequestResponseReceived(bool isError, const QVariantMap &data);

protected:
    virtual QNetworkReply *getRequest(const QString &objectIdentifier, const QString &extraPath, const QStringList &whichFields, const QVariantMap &extraData);
    virtual QNetworkReply *postRequest(const QString &objectIdentifier, const QString &extraPath, const QVariantMap &data, const QVariantMap &extraData);
    virtual QNetworkReply *deleteRequest(const QString &objectIdentifier, const QString &extraPath, const QVariantMap &extraData);
    friend class IdentifiableContentItemInterface;

private:
    bool isInitialized() const;
    friend class ContentItemInterface;

protected:
    SocialNetworkInterface(SocialNetworkInterfacePrivate &dd, QObject *parent = 0);
    QVariantMap contentItemData(ContentItemInterface *contentItem) const;
    void setContentItemData(ContentItemInterface *contentItem, const QVariantMap &data) const;
    virtual ContentItemInterface *contentItemFromData(QObject *parent, const QVariantMap &data) const;
    virtual void updateInternalData(QList<CacheEntry*> data);                         // model data, requires filter/sort/dataChanged()
    virtual void populateDataForNode(IdentifiableContentItemInterface *currentNode);  // requires d->populateCache() + updateInternalData()
    virtual void populateDataForNode(const QString &unseenNodeIdentifier);            // requires d->pushNode(), d->populateCache(),
                                                                                      //     and then updateInternalData()
    QScopedPointer<SocialNetworkInterfacePrivate> d_ptr;

private:
    QList<CacheEntry*> internalData() const;       // this is the model data, which is set via updateInternalData().
    friend class ArbitraryRequestHandler;
    Q_DECLARE_PRIVATE(SocialNetworkInterface)
};

#endif // SOCIALNETWORKINTERFACE_H
