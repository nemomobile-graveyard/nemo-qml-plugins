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

#include "seasidecache.h"
#include "seasideperson.h"

#include <QContactName>
#include <QContactAvatar>

#include <QtDebug>

struct Contact
{
    const char *firstName;
    const char *lastName;
    const char *fullName;
    const char *avatar;
    const bool isFavorite;
};

static const Contact contactsData[] =
{
    { "Aaron", "Aaronson", "Aaron Aaronson", 0, false },            // 0
    { "Aaron", "Arthur", "Aaron Arthur", 0, false },                // 1
    { "Aaron", "Johns", "Aaron Johns", 0, true },                   // 2
    { "Arthur", "Johns", "Arthur Johns", 0, false },                // 3
    { "Jason", "Aaronson", "Jason Aaronson", 0, false },            // 4
    { "Joe", "Johns", "Joe Johns", "file:///cache/joe.jpg", true }, // 5
    { "Robin", "Burchell", "Robin Burchell", 0, true }              // 6
};

SeasideCache *SeasideCache::instance = 0;

SeasideCache::SeasideCache()
{
    instance = this;
    for (int i = 0; i < 3; ++i) {
        m_models[i] = 0;
        m_populated[i] = false;
    }
}

void SeasideCache::reset()
{
    for (int i = 0; i < 3; ++i) {
        m_contacts[i].clear();
        m_populated[i] = false;
        m_models[i] = 0;
    }

    m_cache.clear();

    for (uint i = 0; i < sizeof(contactsData) / sizeof(Contact); ++i) {
        QContact contact;

        QContactId contactId;
        contactId.setLocalId(i);
        contact.setId(contactId);

        QContactName name;
        name.setFirstName(QLatin1String(contactsData[i].firstName));
        name.setLastName(QLatin1String(contactsData[i].lastName));
        name.setCustomLabel(QLatin1String(contactsData[i].fullName));
        contact.saveDetail(&name);

        if (contactsData[i].avatar) {
            QContactAvatar avatar;
            avatar.setImageUrl(QUrl(QLatin1String(contactsData[i].avatar)));
            contact.saveDetail(&avatar);
        }
        m_cache.append(SeasideCacheItem(contact));
    }
}

SeasideCache::~SeasideCache()
{
    instance = 0;
}

void SeasideCache::reference()
{
}

void SeasideCache::release()
{
}

void SeasideCache::registerModel(SeasideFilteredModel *model, SeasideFilteredModel::FilterType type)
{
    for (int i = 0; i < 3; ++i)
        instance->m_models[i] = 0;
    instance->m_models[type] = model;
}

void SeasideCache::unregisterModel(SeasideFilteredModel *)
{
    for (int i = 0; i < 3; ++i)
        instance->m_models[i] = 0;
}

SeasideCacheItem *SeasideCache::cacheItemById(QContactLocalId id)
{
    return &instance->m_cache[id];
}

SeasidePerson *SeasideCache::personById(QContactLocalId id)
{
    return person(&instance->m_cache[id]);
}

QContact SeasideCache::contactById(QContactLocalId id)
{
    return instance->m_cache[id].contact;
}

SeasidePerson *SeasideCache::person(SeasideCacheItem *item)
{
    if (!item->person) {
        item->person = new SeasidePerson(instance);
        item->person->setContact(item->contact);
    }
    return item->person;
}

SeasidePerson *SeasideCache::personByPhoneNumber(const QString &)
{
    return 0;
}

bool SeasideCache::savePerson(SeasidePerson *)
{
    return false;
}

void SeasideCache::removePerson(SeasidePerson *)
{
}

const QVector<QContactLocalId> *SeasideCache::index(SeasideFilteredModel::FilterType filterType)
{
    return &instance->m_contacts[filterType];
}

bool SeasideCache::isPopulated(SeasideFilteredModel::FilterType filterType)
{
    return instance->m_populated[filterType];
}

SeasideFilteredModel::DisplayLabelOrder SeasideCache::displayLabelOrder()
{
    return SeasideFilteredModel::FirstNameFirst;
}

void SeasideCache::populate(SeasideFilteredModel::FilterType filterType)
{
    m_populated[filterType] = true;

    if (m_models[filterType])
        m_models[filterType]->makePopulated();
}

void SeasideCache::insert(SeasideFilteredModel::FilterType filterType, int index, const QVector<QContactLocalId> &ids)
{
    if (m_models[filterType])
        m_models[filterType]->sourceAboutToInsertItems(index, index + ids.count() - 1);

    for (int i = 0; i < ids.count(); ++i)
        m_contacts[filterType].insert(index + i, ids.at(i));

    if (m_models[filterType])
        m_models[filterType]->sourceItemsInserted(index, index + ids.count() - 1);
}

void SeasideCache::remove(SeasideFilteredModel::FilterType filterType, int index, int count)
{
    if (m_models[filterType])
        m_models[filterType]->sourceAboutToRemoveItems(index, index + count - 1);

    m_contacts[filterType].remove(index, count);

    if (m_models[filterType])
        m_models[filterType]->sourceItemsRemoved();
}

int SeasideCache::importContacts(const QString &)
{
    return 0;
}

QString SeasideCache::exportContacts()
{
    return QString();
}

void SeasideCache::setDisplayName(SeasideFilteredModel::FilterType filterType, int index, const QString &displayName)
{
    SeasideCacheItem &cacheItem = m_cache[m_contacts[filterType].at(index)];

    QContactName name = cacheItem.contact.detail<QContactName>();
    name.setCustomLabel(displayName);
    cacheItem.contact.saveDetail(&name);

    cacheItem.filterKey = QStringList();

    if (m_models[filterType])
        m_models[filterType]->sourceDataChanged(index, index);
}
