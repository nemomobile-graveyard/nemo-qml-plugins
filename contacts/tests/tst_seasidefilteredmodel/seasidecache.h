#ifndef SEASIDECACHE_H
#define SEASIDECACHE_H

#include <QContact>

#include "seasidefilteredmodel.h"

struct SeasideCacheItem
{
    SeasideCacheItem() : person(0) {}
    SeasideCacheItem(const QContact &contact) : contact(contact), person(0) {}

    QContact contact;
    SeasidePerson *person;
    QStringList filterKey;
};

class SeasideCache : public QObject
{
    Q_OBJECT
public:
    SeasideCache();
    ~SeasideCache();

    static void registerModel(SeasideFilteredModel *model, SeasideFilteredModel::FilterType type);
    static void unregisterModel(SeasideFilteredModel *model);

    static void registerUser(QObject *user);
    static void unregisterUser(QObject *user);

    static SeasideFilteredModel::DisplayLabelOrder displayLabelOrder();

    static SeasideCacheItem *cacheItemById(QContactLocalId);
    static SeasidePerson *personById(QContactLocalId id);
    static SeasidePerson *selfPerson();
    static QContact contactById(QContactLocalId id);
    static QChar nameGroupForCacheItem(SeasideCacheItem *cacheItem);
    static QList<QChar> allNameGroups();

    static SeasidePerson *person(SeasideCacheItem *item);

    static SeasidePerson *personByPhoneNumber(const QString &msisdn);
    static bool savePerson(SeasidePerson *person);
    static void removePerson(SeasidePerson *person);

    static const QVector<QContactLocalId> *contacts(SeasideFilteredModel::FilterType filterType);
    static bool isPopulated(SeasideFilteredModel::FilterType filterType);

    void populate(SeasideFilteredModel::FilterType filterType);
    void insert(SeasideFilteredModel::FilterType filterType, int index, const QVector<QContactLocalId> &ids);
    void remove(SeasideFilteredModel::FilterType filterType, int index, int count);

    static int importContacts(const QString &path);
    static QString exportContacts();

    void setDisplayName(SeasideFilteredModel::FilterType filterType, int index, const QString &name);

    void reset();

    static QVector<QContactLocalId> getContactsForFilterType(SeasideFilteredModel::FilterType filterType);

    QVector<QContactLocalId> m_contacts[SeasideFilteredModel::FilterTypesCount];
    SeasideFilteredModel *m_models[SeasideFilteredModel::FilterTypesCount];
    bool m_populated[SeasideFilteredModel::FilterTypesCount];

    QVector<SeasideCacheItem> m_cache;

    static SeasideCache *instance;
    static QList<QChar> allContactNameGroups;
};


#endif
