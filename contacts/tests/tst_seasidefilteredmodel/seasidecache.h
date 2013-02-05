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

    static const QVector<QContactLocalId> *index(SeasideFilteredModel::FilterType filterType);
    static bool isPopulated(SeasideFilteredModel::FilterType filterType);

    void populate(SeasideFilteredModel::FilterType filterType);
    void insert(SeasideFilteredModel::FilterType filterType, int index, const QVector<QContactLocalId> &ids);
    void remove(SeasideFilteredModel::FilterType filterType, int index, int count);

    static int importContacts(const QString &path);
    static QString exportContacts();

    void setDisplayName(SeasideFilteredModel::FilterType filterType, int index, const QString &name);

    void reset();



    QVector<QContactLocalId> m_contacts[3];
    SeasideFilteredModel *m_models[3];
    bool m_populated[3];

    QVector<SeasideCacheItem> m_cache;

    static SeasideCache *instance;
};


#endif
