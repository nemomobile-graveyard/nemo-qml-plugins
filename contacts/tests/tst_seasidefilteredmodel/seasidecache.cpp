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
#include <QContactEmailAddress>

#include <QtDebug>

struct Contact
{
    const char *firstName;
    const char *lastName;
    const char *fullName;
    const bool isFavorite;
    const bool isOnline;
    const char *email;
    const char *avatar;

};

static const Contact contactsData[] =
{
/*0*/   { "Aaron",  "Aaronson", "Aaron Aaronson", false, false, "aaronaa@example.com", 0 },
/*1*/   { "Aaron",  "Arthur",   "Aaron Arthur",   false, true,  "aaronar@example.com", 0 },
/*2*/   { "Aaron",  "Johns",    "Aaron Johns",    true,  false, "johns@example.com", 0 },
/*3*/   { "Arthur", "Johns",    "Arthur Johns",   false, true,  "arthur1.johnz@example.org", 0 },
/*4*/   { "Jason",  "Aaronson", "Jason Aaronson", false, false, "jay@examplez.org", 0 },
/*5*/   { "Joe",    "Johns",    "Joe Johns",      true,  true,  "jj@examplez.org", "file:///cache/joe.jpg" },
/*6*/   { "Robin",  "Burchell", "Robin Burchell", true,  false, 0, 0 }
};

static QList<QChar> getAllContactNameGroups()
{
    QList<QChar> groups;
    groups << QLatin1Char('A')
           << QLatin1Char('B')
           << QLatin1Char('C')
           << QLatin1Char('D')
           << QLatin1Char('E')
           << QLatin1Char('F')
           << QLatin1Char('G')
           << QLatin1Char('H')
           << QLatin1Char('I')
           << QLatin1Char('J')
           << QLatin1Char('K')
           << QLatin1Char('L')
           << QLatin1Char('M')
           << QLatin1Char('N')
           << QLatin1Char('O')
           << QLatin1Char('P')
           << QLatin1Char('Q')
           << QLatin1Char('R')
           << QLatin1Char('S')
           << QLatin1Char('T')
           << QLatin1Char('U')
           << QLatin1Char('V')
           << QLatin1Char('W')
           << QLatin1Char('X')
           << QLatin1Char('Y')
           << QLatin1Char('Z')
           << QChar(0x00c5)     // Å
           << QChar(0x00c4)     // Ä
           << QChar(0x00d6)     // Ö
           << QLatin1Char('#');
    return groups;
}

SeasideCache *SeasideCache::instance = 0;
QList<QChar> SeasideCache::allContactNameGroups = getAllContactNameGroups();

SeasideCache::SeasideCache()
{
    instance = this;
    for (int i = 0; i < SeasideFilteredModel::FilterTypesCount; ++i) {
        m_models[i] = 0;
        m_populated[i] = false;
    }
}

void SeasideCache::reset()
{
    for (int i = 0; i < SeasideFilteredModel::FilterTypesCount; ++i) {
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

        if (contactsData[i].email) {
            QContactEmailAddress email;
            email.setEmailAddress(QLatin1String(contactsData[i].email));
            contact.saveDetail(&email);
        }

        m_cache.append(SeasideCacheItem(contact));
    }

    insert(SeasideFilteredModel::FilterAll, 0, getContactsForFilterType(SeasideFilteredModel::FilterAll));
    insert(SeasideFilteredModel::FilterFavorites, 0, getContactsForFilterType(SeasideFilteredModel::FilterFavorites));
    insert(SeasideFilteredModel::FilterOnline, 0, getContactsForFilterType(SeasideFilteredModel::FilterOnline));
}

QVector<QContactLocalId> SeasideCache::getContactsForFilterType(SeasideFilteredModel::FilterType filterType)
{
    QVector<QContactLocalId> ids;

    for (uint i = 0; i < sizeof(contactsData) / sizeof(Contact); ++i) {
        if ((filterType == SeasideFilteredModel::FilterAll) ||
            (filterType == SeasideFilteredModel::FilterFavorites && contactsData[i].isFavorite) ||
            (filterType == SeasideFilteredModel::FilterOnline && contactsData[i].isOnline)) {
            ids.append(i);
        }
    }

    return ids;
}

SeasideCache::~SeasideCache()
{
    instance = 0;
}

void SeasideCache::registerModel(SeasideFilteredModel *model, SeasideFilteredModel::FilterType type)
{
    for (int i = 0; i < SeasideFilteredModel::FilterTypesCount; ++i)
        instance->m_models[i] = 0;
    instance->m_models[type] = model;
}

void SeasideCache::unregisterModel(SeasideFilteredModel *)
{
    for (int i = 0; i < SeasideFilteredModel::FilterTypesCount; ++i)
        instance->m_models[i] = 0;
}

void SeasideCache::registerUser(QObject *)
{
}

void SeasideCache::unregisterUser(QObject *)
{
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

QChar SeasideCache::nameGroupForCacheItem(SeasideCacheItem *cacheItem)
{
    if (!cacheItem)
        return QChar();

    QChar group;
    QString first;
    QString last;
    QContactName nameDetail = cacheItem->contact.detail<QContactName>();
    if (SeasideCache::displayLabelOrder() == SeasideFilteredModel::FirstNameFirst) {
        first = nameDetail.firstName();
        last = nameDetail.lastName();
    } else {
        first = nameDetail.lastName();
        last = nameDetail.firstName();
    }
    if (!first.isEmpty()) {
        group = first[0].toUpper();
    } else if (!last.isEmpty()) {
        group = last[0].toUpper();
    } else {
        QString displayLabel = (cacheItem->person)
                ? cacheItem->person->displayLabel()
                : SeasidePerson::generateDisplayLabel(cacheItem->contact);
        if (!displayLabel.isEmpty())
            group = displayLabel[0].toUpper();
    }

    // XXX temporary workaround for non-latin names: use non-name details to try to find a
    // latin character group
    if (!group.isNull() && group.toLatin1() != group) {
        QString displayLabel = SeasidePerson::generateDisplayLabelFromNonNameDetails(cacheItem->contact);
        if (!displayLabel.isEmpty())
            group = displayLabel[0].toUpper();
    }

    if (group.isNull() || !allContactNameGroups.contains(group)) {
        group = QLatin1Char('#');   // 'other' group
    }
    return group;
}

QList<QChar> SeasideCache::allNameGroups()
{
    return QList<QChar>();
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

SeasidePerson *SeasideCache::selfPerson()
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

const QVector<QContactLocalId> *SeasideCache::contacts(SeasideFilteredModel::FilterType filterType)
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
