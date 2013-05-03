/*
 * Copyright (C) 2012 Robin Burchell <robin+nemo@viroteck.net>
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

#include <QDebug>
#include <QContactLocalIdFilter>
#include <QContactName>

#include <mgconfitem.h>

#include "seasideperson.h"
#include "seasidepeoplemodel_p.h"

#ifdef DEBUG_MODEL
#define MODEL_DEBUG qDebug
#else
#define MODEL_DEBUG if (false) qDebug
#endif

SeasidePeopleModelPriv::SeasidePeopleModelPriv(SeasidePeopleModel *parent)
    : QObject(parent)
    , q(parent)
    , memoryCachePopulated(false)
{
    QContactSortOrder sort;
    sort.setDetailDefinitionName(QContactName::DefinitionName, QContactName::FieldFirstName);
    sort.setDirection(Qt::AscendingOrder);
    sortOrder.clear();
    sortOrder.append(sort);

    MODEL_DEBUG() << Q_FUNC_INFO << QContactManager::availableManagers();
    if (!qgetenv("NEMO_CONTACT_MANAGER").isNull())
        manager = new QContactManager(qgetenv("NEMO_CONTACT_MANAGER"));
    else
        manager = new QContactManager;

    MODEL_DEBUG() << Q_FUNC_INFO << "Manager is " << manager->managerName();

    localeHelper = LocaleUtils::self();

    connect(manager, SIGNAL(contactsAdded(QList<QContactLocalId>)),
            this, SLOT(contactsAdded(QList<QContactLocalId>)));
    connect(manager, SIGNAL(contactsChanged(QList<QContactLocalId>)),
            this, SLOT(contactsChanged(QList<QContactLocalId>)));
    connect(manager, SIGNAL(contactsRemoved(QList<QContactLocalId>)),
            this, SLOT(contactsRemoved(QList<QContactLocalId>)));
    connect(manager, SIGNAL(dataChanged()), this, SLOT(dataReset()));

    dataReset();

    displayLabelOrderConf = new MGConfItem("/org/nemomobile/contacts/display_label_order", this);
    connect(displayLabelOrderConf, SIGNAL(valueChanged()), this, SLOT(onDisplayLabelOrderConfChanged()));
}

QString SeasidePeopleModelPriv::normalizePhoneNumber(const QString &msisdn)
{
    // TODO:Possibly be more efficient here.
    QString normalized = msisdn;
    normalized.replace(QRegExp("[^0-9]"), "");
    return normalized.right(7);
}

void SeasidePeopleModelPriv::addContacts(const QList<QContact> contactsList, int size)
{
    foreach (const QContact &contact, contactsList) {
        SeasidePerson *person = new SeasidePerson(contact);
        //MODEL_DEBUG() << Q_FUNC_INFO << "Adding contact " << contact.id() << " local " << contact.localId();
        QContactLocalId id = contact.localId();

        // Make sure we don't duplicate contacts
        if (!idToIndex.contains(id)) {
          contactIds.push_back(id);
          idToIndex.insert(id, size++);
        }
        idToContact.insert(id, person);

        // Create normalized numbers -> contact id map.
        foreach(const QString &sourcePhoneNumber, person->phoneNumbers()) {
            QString normalizedPhoneNumber = SeasidePeopleModelPriv::normalizePhoneNumber(sourcePhoneNumber);
            if(phoneNumbersToContactIds.contains(normalizedPhoneNumber)) continue; // Ignore duplicates, first-come-first-serve.

            phoneNumbersToContactIds.insert(normalizedPhoneNumber, contact.localId());
        }

        connect(q, SIGNAL(displayLabelOrderChanged(SeasideProxyModel::DisplayLabelOrder)),
                person, SLOT(recalculateDisplayLabel(SeasideProxyModel::DisplayLabelOrder)), Qt::UniqueConnection);

        if (q->displayLabelOrder() != personDisplayLabelOrder) {
            person->recalculateDisplayLabel(q->displayLabelOrder());
        }
    }

    personDisplayLabelOrder = q->displayLabelOrder();
}

void SeasidePeopleModelPriv::fixIndexMap()
{
    int i=0;
    q->beginResetModel();
    idToIndex.clear();
    foreach (const QContactLocalId& id, contactIds)
        idToIndex.insert(id, i++);
    q->endResetModel();
}

void SeasidePeopleModelPriv::savePendingContacts()
{
    QContactSaveRequest *saveRequest = new QContactSaveRequest(this);
    connect(saveRequest,
            SIGNAL(stateChanged(QContactAbstractRequest::State)),
            SLOT(onSaveStateChanged(QContactAbstractRequest::State)));
    saveRequest->setContacts(contactsPendingSave);
    saveRequest->setManager(manager);

    foreach (const QContact &contact, contactsPendingSave)
        MODEL_DEBUG() << Q_FUNC_INFO << "Saving " << contact.id();

    if (!saveRequest->start()) {
        qWarning() << Q_FUNC_INFO << "Save request failed: " << saveRequest->error();
        delete saveRequest;
    }

    contactsPendingSave.clear();
}

template<typename T> inline T *checkRequest(QObject *sender, QContactAbstractRequest::State requestState)
{
    MODEL_DEBUG() << Q_FUNC_INFO << "Request state: " << requestState;
    T *request = qobject_cast<T *>(sender);
    if (!request) {
        qWarning() << Q_FUNC_INFO << "NULL request pointer";
        return 0;
    }

    if (request->error() != QContactManager::NoError) {
        qWarning() << Q_FUNC_INFO << "Error" << request->error()
                 << "occurred during request!";
        request->deleteLater();
        return 0;
    }

    if (requestState != QContactAbstractRequest::FinishedState &&
        requestState != QContactAbstractRequest::CanceledState)
    {
        // ignore
        return 0;
    }

    return request;
}

void SeasidePeopleModelPriv::onSaveStateChanged(QContactAbstractRequest::State requestState)
{
    QContactSaveRequest *saveRequest = checkRequest<QContactSaveRequest>(sender(), requestState);
    if (!saveRequest)
        return;

    QList<QContact> contactList = saveRequest->contacts();
    QList<QContact> newContacts;

    foreach (const QContact &new_contact, contactList) {
        MODEL_DEBUG() << Q_FUNC_INFO << "Successfully saved " << new_contact.id();

        // make sure data shown to user matches what is
        // really in the database
        QContactLocalId id = new_contact.localId();
        SeasidePerson *person = q->personById(id);
        if (person)
            person->setContact(new_contact);
        else
            newContacts.append(new_contact);
    }

    int size = contactIds.size();

    if (newContacts.size()) {
        q->beginInsertRows(QModelIndex(), size, size + newContacts.size() - 1);
        addContacts(newContacts, size);
        q->endInsertRows();
    }

    saveRequest->deleteLater();
}

void SeasidePeopleModelPriv::onRemoveStateChanged(QContactAbstractRequest::State requestState)
{
    QContactRemoveRequest *removeRequest = checkRequest<QContactRemoveRequest>(sender(), requestState);
    if (!removeRequest)
        return;

    MODEL_DEBUG() << Q_FUNC_INFO << "Removed" << removeRequest->contactIds();
    removeRequest->deleteLater();
    // actual removal done on contactsRemoved signal
}

// helper function to check validity of sender and stuff.
void SeasidePeopleModelPriv::contactsAdded(const QList<QContactLocalId>& contactIds)
{
    if (contactIds.size() == 0)
        return;

    // if we saved the contact, then we'll already have them in the model, and don't
    // need to fetch them again. In fact, doing so is not only a waste, it'll cause
    // a crash when processing the fetch, as we'll beginInsertRows, and then not really
    // insert that many rows.
    QList<QContactLocalId> filteredLocalIds;
    filteredLocalIds.reserve(contactIds.size());

    foreach (const QContactLocalId &id, contactIds) {
        if (!q->personById(id))
            filteredLocalIds.append(id);
    }

    if (!filteredLocalIds.count())
        return; // no sense fetching nothing

    QContactLocalIdFilter filter;
    filter.setIds(filteredLocalIds);

    QContactFetchRequest *fetchRequest = new QContactFetchRequest(this);
    fetchRequest->setManager(manager);
    connect(fetchRequest,
            SIGNAL(stateChanged(QContactAbstractRequest::State)),
            SLOT(onAddedFetchChanged(QContactAbstractRequest::State)));
    fetchRequest->setFilter(filter);
    MODEL_DEBUG() << Q_FUNC_INFO << "Fetching new contacts " << filteredLocalIds;

    if (!fetchRequest->start()) {
        qWarning() << Q_FUNC_INFO << "Fetch request failed";
        delete fetchRequest;
        return;
    }
}

void SeasidePeopleModelPriv::onAddedFetchChanged(QContactAbstractRequest::State requestState)
{
    QContactFetchRequest *fetchRequest = checkRequest<QContactFetchRequest>(sender(), requestState);
    if (!fetchRequest)
        return;

    QList<QContact> addedContactsList = fetchRequest->contacts();

    int size = contactIds.size();
    int added = addedContactsList.size();

    q->beginInsertRows(QModelIndex(), size, size + added - 1);
    addContacts(addedContactsList, size);
    q->endInsertRows();

    MODEL_DEBUG() << Q_FUNC_INFO << "Done updating model after adding"
        << added << "contacts";
    fetchRequest->deleteLater();
}

void SeasidePeopleModelPriv::contactsChanged(const QList<QContactLocalId>& contactIds)
{
    if (contactIds.size() == 0)
        return;

    QContactLocalIdFilter filter;
    filter.setIds(contactIds);

    QContactFetchRequest *fetchRequest = new QContactFetchRequest(this);
    fetchRequest->setManager(manager);
    connect(fetchRequest,
            SIGNAL(stateChanged(QContactAbstractRequest::State)),
            SLOT(onChangedFetchChanged(QContactAbstractRequest::State)));
    fetchRequest->setFilter(filter);

    MODEL_DEBUG() << Q_FUNC_INFO << "Fetching changed contacts " << contactIds;

    if (!fetchRequest->start()) {
        qWarning() << Q_FUNC_INFO << "Fetch request failed";
        delete fetchRequest;
        return;
    }
}

void SeasidePeopleModelPriv::onDisplayLabelOrderConfChanged()
{
    emit q->displayLabelOrderChanged((SeasideProxyModel::DisplayLabelOrder)displayLabelOrderConf->value().toInt());
}

void SeasidePeopleModelPriv::onChangedFetchChanged(QContactAbstractRequest::State requestState)
{
    QContactFetchRequest *fetchRequest = checkRequest<QContactFetchRequest>(sender(), requestState);
    if (!fetchRequest)
        return;

    // NOTE: this implementation sends one dataChanged signal with
    // the minimal range that covers all the changed contacts, but it
    // could be more efficient to send multiple dataChanged signals,
    // though more work to find them
    int min = contactIds.size();
    int max = 0;

    QList<QContact> changedContactsList = fetchRequest->contacts();

    foreach (const QContact &changedContact, changedContactsList) {
        MODEL_DEBUG() << Q_FUNC_INFO << "Fetched changed contact " << changedContact.id();
        QContactLocalId id = changedContact.localId();

        QMap<QContactLocalId, int>::ConstIterator it = idToIndex.find(changedContact.localId());
        if (it == idToIndex.constEnd())
            continue;

        int index = *it;

        if (index < min)
            min = index;

        if (index > max)
            max = index;

        idToContact[id]->setContact(changedContact);

        // TODO: use normalized number from qtcontacts-tracker/mobility
        SeasidePerson *person = idToContact[id];
        foreach(const QString &msisdn, person->phoneNumbers()) {
            QString normalizedPhoneNumber = SeasidePeopleModelPriv::normalizePhoneNumber(msisdn);

            if(!phoneNumbersToContactIds.contains(normalizedPhoneNumber)) {
                phoneNumbersToContactIds.insert(normalizedPhoneNumber, id);
            }
        }

        foreach(const QString &phoneNumber, phoneNumbersToContactIds.keys()) {
           bool remove = true;
           foreach(const QString &msisdn, person->phoneNumbers()) {
               QString normalizedPhoneNumber = SeasidePeopleModelPriv::normalizePhoneNumber(msisdn);
               if(phoneNumber == normalizedPhoneNumber) {
                   remove = false;
                   break;
               }
           }
           if(remove) {
               phoneNumbersToContactIds.remove(phoneNumber);
           }
        }
    }

    // FIXME: unfortunate that we can't easily identify what changed
    if (min <= max)
        emit q->dataChanged(q->index(min, 0), q->index(max, 0));

    MODEL_DEBUG() << Q_FUNC_INFO << "Done updating model after contacts update";
    fetchRequest->deleteLater();
}

void SeasidePeopleModelPriv::contactsRemoved(const QList<QContactLocalId>& contactIds)
{
    MODEL_DEBUG() << Q_FUNC_INFO << "contacts removed:" << contactIds;

    QList<int> removed;
    removed.reserve(contactIds.size());

    foreach (const QContactLocalId& id, contactIds) {
        QMap<QContactLocalId, int>::ConstIterator it = idToIndex.find(id);
        if (it == idToIndex.end())
            continue;

        removed.push_front(*it);
    }
    std::sort(removed.begin(), removed.end());

    // NOTE: this could check for adjacent rows being removed and send fewer signals
    int size = removed.size();
    for (int i=0; i<size; i++) {
        // remove in reverse order so the other index numbers will not change
        int index = removed.takeLast();
        q->beginRemoveRows(QModelIndex(), index, index);
        QContactLocalId id = this->contactIds.takeAt(index);

        // don't use delete here, otherwise it'll break the fake removal of
        // contacts in the case of the memory manager (remove request -> instant
        // non-queued contactsRemoved -> gets us back here again, so we delete
        // the contact before we've had a chance to fake the removal)
        idToContact.take(id)->deleteLater();
        idToIndex.remove(id);

        foreach(const QString &phoneNumber, phoneNumbersToContactIds.keys()) {
            QContactLocalId cId = phoneNumbersToContactIds.value(phoneNumber);
            if(cId == id) phoneNumbersToContactIds.remove(phoneNumber);
        }

        q->endRemoveRows();
    }
    fixIndexMap();
}

void SeasidePeopleModelPriv::dataReset()
{
    MODEL_DEBUG() << Q_FUNC_INFO << "data reset";
    QContactFetchRequest *fetchRequest = new QContactFetchRequest(this);
    fetchRequest->setManager(manager);
    connect(fetchRequest,
            SIGNAL(stateChanged(QContactAbstractRequest::State)),
            SLOT(onDataResetFetchChanged(QContactAbstractRequest::State)));
    fetchRequest->setFilter(currentFilter);

    if (!fetchRequest->start()) {
        qWarning() << Q_FUNC_INFO << "Fetch request failed";
        delete fetchRequest;
        return;
    }
}

void SeasidePeopleModelPriv::onDataResetFetchChanged(QContactAbstractRequest::State requestState)
{
    QContactFetchRequest *fetchRequest = checkRequest<QContactFetchRequest>(sender(), requestState);
    if (!fetchRequest)
        return;

    QList<QContact> contactsList = fetchRequest->contacts();
    contactsList.append(manager->contact(manager->selfContactId()));
    int size = 0;

    MODEL_DEBUG() << Q_FUNC_INFO << "Starting model reset";
    q->beginResetModel();

    contactIds.clear();
    qDeleteAll(idToContact);
    idToContact.clear();
    idToIndex.clear();
    phoneNumbersToContactIds.clear();

    addContacts(contactsList, size);

    memoryCachePopulated = true;
    q->endResetModel();
    MODEL_DEBUG() << Q_FUNC_INFO << "Done with model reset";
    fetchRequest->deleteLater();
    q->populatedChanged();
}




#if 0
// graveyard of code that might come in handy


    enum FilterRoles{
        AllFilter = 0,
        FavoritesFilter,
        OnlineFilter,
        ContactFilter
    };


// TODO: can't have the slot on public API, so disabled until we split to a
// private class
void SeasidePeopleModel::exportContact(QString uuid,  QString filename){
    QVersitContactExporter exporter;
    QList<QContact> contacts;
    QList<QVersitDocument> documents;

    QContactLocalId id = uuidToId[uuid];
    QContact &person = idToContact[id];

    if(person.isEmpty()){
        qWarning() << "[SeasidePeopleModel] no contact found to export with uuid " + uuid;
        return;
    }

    contacts.append(person);
    exporter.exportContacts(contacts);
    documents = exporter.documents();

    QFile * file = new QFile(filename);
    if(file->open(QIODevice::ReadWrite)){
        writer.setDevice(file);
        writer.startWriting(documents);
    }else{
        qWarning() << "[SeasidePeopleModel] vCard export failed for contact with uuid " + uuid;
    }
}

void SeasidePeopleModel::vCardFinished(QVersitWriter::State state)
{
    if(state == QVersitWriter::FinishedState || state == QVersitWriter::CanceledState){
        delete writer.device();
        writer.setDevice(0);
    }
}

void SeasidePeopleModel::setSorting(int role){
    QContactSortOrder sort;

    switch(role){
    case LastNameRole:
        sort.setDetailDefinitionName(QContactName::DefinitionName, 
                                     QContactName::FieldLastName);
        break;
    case FirstNameRole:
    default:
        sort.setDetailDefinitionName(QContactName::DefinitionName, 
                                     QContactName::FieldFirstName);
        break;
    }

    sort.setDirection(Qt::AscendingOrder);
    sortOrder.clear();
    sortOrder.append(sort);
}

int SeasidePeopleModel::getSortingRole(){
    if ((sortOrder.isEmpty()) ||
       (sortOrder.at(0).detailFieldName() == QContactName::FieldFirstName))
        return SeasidePeopleModel::FirstNameRole;
    else if (sortOrder.at(0).detailFieldName() == QContactName::FieldLastName)
        return SeasidePeopleModel::LastNameRole;

    return SeasidePeopleModel::FirstNameRole;
}

void SeasidePeopleModel::setFilter(int role, bool dataResetNeeded){
    switch(role){
    case FavoritesFilter:
    {
        QContactDetailFilter favFilter;
        favFilter.setDetailDefinitionName(QContactFavorite::DefinitionName, QContactFavorite::FieldFavorite);
        favFilter.setValue(true);
        currentFilter = favFilter;
        break;
    }
    case OnlineFilter:
    {
        QContactDetailFilter availableFilter;
        availableFilter.setDetailDefinitionName(QContactPresence::DefinitionName, QContactPresence::FieldPresenceState);
        availableFilter.setValue(QContactPresence::PresenceAvailable);
        currentFilter = availableFilter;
        break;
    }
    case AllFilter:
    {
        currentFilter = QContactFilter();
        break;
    }
    case ContactFilter:
    {
        QContactDetailFilter contactFilter;
        contactFilter.setDetailDefinitionName(QContactGuid::DefinitionName, QContactGuid::FieldGuid);
        contactFilter.setValue(currentGuid.guid());
        currentFilter = contactFilter;
        break;
    }
    default:
    {
        currentFilter = QContactFilter();
        break;
    }
    }

    if (dataResetNeeded)
        dataReset();
}

void SeasidePeopleModel::searchContacts(const QString text){

        MODEL_DEBUG() << "[SeasidePeopleModel] searchContact " + text;
        QList<QContactFilter> filterList;
        QContactUnionFilter unionFilter;

        QContactDetailFilter nameFilter;
        nameFilter.setDetailDefinitionName(QContactName::DefinitionName, QContactName::FieldFirstName);
        nameFilter.setValue(text);
        nameFilter.setMatchFlags(QContactFilter::MatchContains);
        filterList.append(nameFilter);

        QContactDetailFilter lastFilter;
        lastFilter.setDetailDefinitionName(QContactName::DefinitionName, QContactName::FieldLastName);
        lastFilter.setValue(text);
        lastFilter.setMatchFlags(QContactFilter::MatchContains);
        filterList.append(lastFilter);

        QContactDetailFilter companyFilter;
        companyFilter.setDetailDefinitionName(QContactOrganization::DefinitionName, QContactOrganization::FieldName);
        companyFilter.setValue(text);
        companyFilter.setMatchFlags(QContactFilter::MatchContains);
        filterList.append(companyFilter);

        //removes (). checks last 7 numbers
        QContactDetailFilter phoneNumFilter;
        phoneNumFilter.setDetailDefinitionName(QContactPhoneNumber::DefinitionName, QContactPhoneNumber::FieldNumber);
        phoneNumFilter.setValue(text);
        phoneNumFilter.setMatchFlags(QContactFilter::MatchPhoneNumber);
        filterList.append(phoneNumFilter);

        //checks for contains only
        QContactDetailFilter phoneFilter;
        phoneFilter.setDetailDefinitionName(QContactPhoneNumber::DefinitionName, QContactPhoneNumber::FieldNumber);
        phoneFilter.setValue(text);
        phoneFilter.setMatchFlags(QContactFilter::MatchContains);
        filterList.append(phoneFilter);

        QContactDetailFilter emailFilter;
        emailFilter.setDetailDefinitionName(QContactEmailAddress::DefinitionName, QContactEmailAddress::FieldEmailAddress);
        emailFilter.setValue(text);
        emailFilter.setMatchFlags(QContactFilter::MatchContains);
        filterList.append(emailFilter);

        QContactDetailFilter addressFilter;
        addressFilter.setDetailDefinitionName(QContactAddress::DefinitionName, QContactAddress::FieldStreet);
        addressFilter.setValue(text);
        addressFilter.setMatchFlags(QContactFilter::MatchContains);
        filterList.append(addressFilter);

        QContactDetailFilter countryFilter;
        countryFilter.setDetailDefinitionName(QContactAddress::DefinitionName, QContactAddress::FieldCountry);
        countryFilter.setValue(text);
        countryFilter.setMatchFlags(QContactFilter::MatchContains);
        filterList.append(countryFilter);

        QContactDetailFilter localeFilter;
        localeFilter.setDetailDefinitionName(QContactAddress::DefinitionName, QContactAddress::FieldLocality);
        localeFilter.setValue(text);
        localeFilter.setMatchFlags(QContactFilter::MatchContains);
        filterList.append(localeFilter);

        QContactDetailFilter zipFilter;
        zipFilter.setDetailDefinitionName(QContactAddress::DefinitionName, QContactAddress::FieldPostcode);
        zipFilter.setValue(text);
        zipFilter.setMatchFlags(QContactFilter::MatchContains);
        filterList.append(zipFilter);

        QContactDetailFilter regionFilter;
        regionFilter.setDetailDefinitionName(QContactAddress::DefinitionName, QContactAddress::FieldRegion);
        regionFilter.setValue(text);
        regionFilter.setMatchFlags(QContactFilter::MatchContains);
        filterList.append(regionFilter);

        QContactDetailFilter urlFilter;
        urlFilter.setDetailDefinitionName(QContactUrl::DefinitionName);
        urlFilter.setValue(text);
        urlFilter.setMatchFlags(QContactFilter::MatchExactly);
        filterList.append(urlFilter);

        unionFilter.setFilters(filterList);
        currentFilter = unionFilter;
        dataReset();
}

void SeasidePeopleModel::clearSearch(){
     setFilter(AllFilter);
}


#endif
