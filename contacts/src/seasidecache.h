/*
 * Copyright (C) 2013 Jolla Mobile <andrew.den.exter@jollamobile.com>
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

#ifndef SEASIDECACHE_H
#define SEASIDECACHE_H

#include <QContact>
#include <QContactManager>
#include <QContactLocalIdFilter>
#include <QContactFetchRequest>
#include <QContactRemoveRequest>
#include <QContactSaveRequest>

#include <QBasicTimer>
#include <QSet>

#include <QElapsedTimer>

#ifdef HAS_MLITE
#include <mgconfitem.h>
#endif

#ifdef SEASIDE_SPARQL_QUERIES
#include "sparqlfetchrequest_p.h"
#else
#include <QContactLocalIdFetchRequest>
#endif

#include "seasidefilteredmodel.h"

struct SeasideCacheItem
{
    SeasideCacheItem() : person(0), hasCompleteContact(false) {}
    SeasideCacheItem(const QContact &contact) : contact(contact), person(0), hasCompleteContact(false) {}

    QContact contact;
    SeasidePerson *person;
    QStringList filterKey;
    bool hasCompleteContact;
};

class SeasideVersitExport;
class SeasideVersitImport;

class SeasideCache : public QObject
{
    Q_OBJECT
public:
    static void reference();
    static void release();

    static void registerModel(SeasideFilteredModel *model, SeasideFilteredModel::FilterType type);
    static void unregisterModel(SeasideFilteredModel *model);

    static SeasideFilteredModel::DisplayLabelOrder displayLabelOrder();

    static SeasideCacheItem *cacheItemById(QContactLocalId);
    static SeasidePerson *personById(QContactLocalId id);
    static QContact contactById(QContactLocalId id);

    static SeasidePerson *person(SeasideCacheItem *item);

    static SeasidePerson *personByPhoneNumber(const QString &msisdn);
    static bool savePerson(SeasidePerson *person);
    static void removePerson(SeasidePerson *person);

    static void createContacts(const QList<QContact> &contacts, SeasideVersitImport *importer);
    static void populateCache(
            SeasideVersitExport *exporter, const QList<QContactLocalId> &contactIds);

    static void cancelImport(SeasideVersitImport *importer);
    static void cancelExport(SeasideVersitExport *exporter);

    static QList<QContact> contacts(const QList<QContactLocalId> &contactIds);

    static int importContacts(const QString &path);
    static QString exportContacts();

    static const QVector<QContactLocalId> *index(SeasideFilteredModel::FilterType filterType);
    static bool isPopulated(SeasideFilteredModel::FilterType filterType);

    bool event(QEvent *event);

    // For synchronizeLists()
    int insertRange(int index, int count, const QList<QContactLocalId> &source, int sourceIndex) {
        return insertRange(m_fetchFilter, index, count, source, sourceIndex); }
    int removeRange(int index, int count) { removeRange(m_fetchFilter, index, count); return 0; }

protected:
    void timerEvent(QTimerEvent *event);

private slots:
    void contactsAvailable();
    void contactIdsAvailable();
    void requestStateChanged(QContactAbstractRequest::State state);
    void contactsRemoved(const QList<QContactLocalId> &contactIds);
    void updateContacts();
    void updateContacts(const QList<QContactLocalId> &contactIds);
    void displayLabelOrderChanged();

private:
    SeasideCache();
    ~SeasideCache();

    void requestUpdate();
    void appendContacts(const QList<QContact> &contacts);

    void finalizeUpdate(SeasideFilteredModel::FilterType filter);
    void removeRange(SeasideFilteredModel::FilterType filter, int index, int count);
    int insertRange(
            SeasideFilteredModel::FilterType filter,
            int index,
            int count,
            const QList<QContactLocalId> &queryIds,
            int queryIndex);

    void updateContactData(QContactLocalId contactId, SeasideFilteredModel::FilterType filter);
    void removeContactData(QContactLocalId contactId, SeasideFilteredModel::FilterType filter);
    void makePopulated(SeasideFilteredModel::FilterType filter);

    enum {
        AllPopulated       = 0x3,
        FavoritesPopulated = 0x4
    };

    QBasicTimer m_expiryTimer;
    QHash<QContactLocalId, SeasideCacheItem> m_people;
    QHash<QString, QContactLocalId> m_phoneNumberIds;
    QHash<QContactLocalId, QContact> m_contactsToSave;
    QList<QContact> m_contactsToCreate;
    QList<QContactLocalId> m_contactsToRemove;
    QList<QContactLocalId> m_changedContacts;
    QVector<QContactLocalId> m_index[3];
    QList<SeasideFilteredModel *> m_models[3];
    QList<SeasideVersitExport *> m_pendingExports;
    QList<SeasideVersitImport *> m_pendingImports;
    QHash<QContactLocalId,int> m_expiredContacts;
    QContactManager m_manager;
    QContactFetchRequest m_fetchRequest;
#ifdef SEASIDE_SPARQL_QUERIES
    SparqlFetchRequest m_contactIdRequest;
#else
    QContactLocalIdFetchRequest m_contactIdRequest;
#endif
    QContactRemoveRequest m_removeRequest;
    QContactSaveRequest m_saveRequest;
#ifdef HAS_MLITE
    MGConfItem m_displayLabelOrderConf;
#endif
    int m_refCount;
    int m_resultsRead;
    int m_populated;
    int m_cacheIndex;
    int m_queryIndex;
    QContactLocalId m_selfId;
    SeasideFilteredModel::FilterType m_fetchFilter;
    SeasideFilteredModel::DisplayLabelOrder m_displayLabelOrder;
    bool m_updatesPending;
    bool m_fetchActive;
    bool m_refreshRequired;

    QElapsedTimer m_timer;

    static SeasideCache *instance;
};


#endif
