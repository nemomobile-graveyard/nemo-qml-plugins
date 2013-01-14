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

#include <QContactAvatar>
#include <QContactBirthday>
#include <QContactAnniversary>
#include <QContactName>
#include <QContactNickname>
#include <QContactFavorite>
#include <QContactPhoneNumber>
#include <QContactEmailAddress>
#include <QContactAddress>
#include <QContactOnlineAccount>
#include <QContactOrganization>
#include <QContactUrl>

#include "seasideperson.h"

SeasidePerson::SeasidePerson(QObject *parent)
    : QObject(parent)
{

}

SeasidePerson::SeasidePerson(const QContact &contact, QObject *parent)
    : QObject(parent)
    , mContact(contact)
{
    recalculateDisplayLabel();
}

SeasidePerson::~SeasidePerson()
{
    emit contactRemoved();
}

// QT5: this needs to change type
int SeasidePerson::id() const
{
    return mContact.id().localId();
}

bool SeasidePerson::hasDetail(DetailType detail) const
{
    switch (detail) {
    case FirstNameType:
        return mContact.detail<QContactName>().hasValue(QContactName::FieldFirstName);
    case LastNameType:
        return mContact.detail<QContactName>().hasValue(QContactName::FieldLastName);
    case MiddleNameType:
        return mContact.detail<QContactName>().hasValue(QContactName::FieldMiddleName);
    case CompanyType:
        return mContact.detail<QContactOrganization>().hasValue(QContactOrganization::FieldName);
    case TitleType:
        return mContact.detail<QContactName>().hasValue(QContactName::FieldPrefix);
    case NickType:
        return mContact.detail<QContactNickname>().hasValue(QContactNickname::FieldNickname);
    case PhoneHomeType:
        return hasPhoneType(QContactDetail::ContextHome, QContactPhoneNumber::SubTypeLandline);
    case PhoneWorkType:
        return hasPhoneType(QContactDetail::ContextWork, QContactPhoneNumber::SubTypeLandline);
    case PhoneMobileType:
        return hasPhoneType(QContactDetail::ContextHome, QContactPhoneNumber::SubTypeMobile);
    case PhoneFaxType:
        return hasPhoneType(QContactDetail::ContextHome, QContactPhoneNumber::SubTypeFax);
    case PhonePagerType:
        return hasPhoneType(QContactDetail::ContextHome, QContactPhoneNumber::SubTypePager);
    case EmailHomeType:
        return hasEmailType(QContactDetail::ContextHome);
    case EmailWorkType:
        return hasEmailType(QContactDetail::ContextWork);
    case EmailOtherType:
        return hasEmailType(QContactDetail::ContextOther);
    case AddressHomeType:
        return hasAddressType(QContactDetail::ContextHome);
    case AddressWorkType:
        return hasAddressType(QContactDetail::ContextWork);
    case AddressOtherType:
        return hasAddressType(QContactDetail::ContextOther);
    case WebsiteHomeType:
        return hasWebsiteType(QContactDetail::ContextHome);
    case WebsiteWorkType:
        return hasWebsiteType(QContactDetail::ContextWork);
    case WebsiteOtherType:
        return hasWebsiteType(QContactDetail::ContextOther);
    case BirthdayType:
        return mContact.detail<QContactBirthday>().hasValue(QContactBirthday::FieldBirthday);
    case AnniversaryType:
        return mContact.detail<QContactAnniversary>().hasValue(QContactAnniversary::FieldOriginalDate);

    // XXX move these address types to an "address sub-type" enum
    case AddressStreetType:
    case AddressLocalityType:
    case AddressRegionType:
    case AddressPostcodeType:
    case AddressCountryType:
    case AddressPOBoxType:
        return false;
    }

    return false;
}

QString SeasidePerson::firstName() const
{
    QContactName nameDetail = mContact.detail<QContactName>();
    return nameDetail.firstName();
}

void SeasidePerson::setFirstName(const QString &name)
{
    QContactName nameDetail = mContact.detail<QContactName>();
    nameDetail.setFirstName(name);
    mContact.saveDetail(&nameDetail);
    emit firstNameChanged();
    recalculateDisplayLabel();
}

QString SeasidePerson::lastName() const
{
    QContactName nameDetail = mContact.detail<QContactName>();
    return nameDetail.lastName();
}

void SeasidePerson::setLastName(const QString &name)
{
    QContactName nameDetail = mContact.detail<QContactName>();
    nameDetail.setLastName(name);
    mContact.saveDetail(&nameDetail);
    emit lastNameChanged();
    recalculateDisplayLabel();
}

QString SeasidePerson::middleName() const
{
    QContactName nameDetail = mContact.detail<QContactName>();
    return nameDetail.middleName();
}

void SeasidePerson::setMiddleName(const QString &name)
{
    QContactName nameDetail = mContact.detail<QContactName>();
    nameDetail.setMiddleName(name);
    mContact.saveDetail(&nameDetail);
    emit middleNameChanged();
    recalculateDisplayLabel();
}

// small helper to avoid inconvenience
static QString generateDisplayLabel(QContact mContact)
{
    //REVISIT: Move this or parts of this to localeutils.cpp
    QString displayLabel;
    QContactName name = mContact.detail<QContactName>();
    QString nameStr1 = name.firstName();
    QString nameStr2 = name.middleName();
    QString nameStr3 = name.lastName();

    if (!nameStr1.isEmpty())
        displayLabel.append(nameStr1);

    if (nameStr1.isEmpty() && !nameStr2.isEmpty()) {
        if (!displayLabel.isEmpty())
            displayLabel.append(" ");
        displayLabel.append(nameStr2);
    }

    if (!nameStr3.isEmpty()) {
        if (!displayLabel.isEmpty())
            displayLabel.append(" ");
        displayLabel.append(nameStr3);
    }

    if (!displayLabel.isEmpty())
        return displayLabel;

    foreach (const QContactOnlineAccount& account, mContact.details<QContactOnlineAccount>()) {
        if (!account.accountUri().isNull())
            return account.accountUri();
    }

    foreach (const QContactEmailAddress& email, mContact.details<QContactEmailAddress>()) {
        if (!email.emailAddress().isNull())
            return email.emailAddress();
    }

    QContactOrganization company = mContact.detail<QContactOrganization>();
    if (!company.name().isNull())
        return company.name();

    foreach (const QContactPhoneNumber& phone, mContact.details<QContactPhoneNumber>()) {
        if (!phone.number().isNull())
            return phone.number();
    }

    return "(unnamed)"; // TODO: localisation
}

void SeasidePerson::recalculateDisplayLabel()
{
    // prefer user-set nickname over generated display label
    QString nick = nickname();
    QString oldDisplayLabel = mDisplayLabel;
    QString newDisplayLabel = nick.isEmpty() ? generateDisplayLabel(mContact) : nick;

    if (oldDisplayLabel != newDisplayLabel) {
        mDisplayLabel = newDisplayLabel;
        emit displayLabelChanged();
    }
}

QString SeasidePerson::displayLabel() const
{
    return mDisplayLabel;
}

QString SeasidePerson::sectionBucket() const
{
    if (displayLabel().isEmpty())
        return QString();

//    return priv->localeHelper->getBinForString(data(row,
//                DisplayLabelRole).toString());
    // TODO: won't be at all correct for localisation
    // for some locales (asian in particular), we may need multiple bytes - not
    // just the first - also, we should use QLocale (or ICU) to uppercase, not
    // QString, as QString uses C locale.
    return displayLabel().at(0).toUpper();

}

QString SeasidePerson::companyName() const
{
    QContactOrganization company = mContact.detail<QContactOrganization>();
    return company.name();
}

void SeasidePerson::setCompanyName(const QString &name)
{
    QContactOrganization companyNameDetail = mContact.detail<QContactOrganization>();
    companyNameDetail.setName(name);
    mContact.saveDetail(&companyNameDetail);
    emit companyNameChanged();
}

QString SeasidePerson::nickname() const
{
    QContactNickname nameDetail = mContact.detail<QContactNickname>();
    return nameDetail.nickname();
}

void SeasidePerson::setNickname(const QString &name)
{
    QContactNickname nameDetail = mContact.detail<QContactNickname>();
    nameDetail.setNickname(name);
    mContact.saveDetail(&nameDetail);
    emit nicknameChanged();
    recalculateDisplayLabel();
}

QString SeasidePerson::title() const
{
    QContactName nameDetail = mContact.detail<QContactName>();
    return nameDetail.prefix();
}

void SeasidePerson::setTitle(const QString &name)
{
    QContactName nameDetail = mContact.detail<QContactName>();
    nameDetail.setPrefix(name);
    mContact.saveDetail(&nameDetail);
    emit titleChanged();
}

bool SeasidePerson::favorite() const
{
    QContactFavorite favoriteDetail = mContact.detail<QContactFavorite>();
    return favoriteDetail.isFavorite();
}

void SeasidePerson::setFavorite(bool favorite)
{
    QContactFavorite favoriteDetail = mContact.detail<QContactFavorite>();
    favoriteDetail.setFavorite(favorite);
    mContact.saveDetail(&favoriteDetail);
    emit favoriteChanged();
}

QUrl SeasidePerson::avatarPath() const
{
    QContactAvatar avatarDetail = mContact.detail<QContactAvatar>();
    QUrl avatarUrl = avatarDetail.imageUrl();
    if (avatarUrl.isEmpty())
        return QUrl("image://theme/icon-m-telephony-contact-avatar");
    return avatarUrl;
}

void SeasidePerson::setAvatarPath(QUrl avatarPath)
{
    QContactAvatar avatarDetail = mContact.detail<QContactAvatar>();
    avatarDetail.setImageUrl(QUrl(avatarPath));
    mContact.saveDetail(&avatarDetail);
    emit avatarPathChanged();
}

#define LIST_PROPERTY_FROM_DETAIL_FIELD(detailType, fieldName) \
    QStringList list; \
    \
    foreach (const detailType &detail, mContact.details<detailType>()) { \
        if (!detail.fieldName().isEmpty()) \
            list << detail.fieldName(); \
    } \
    return list;

QStringList SeasidePerson::phoneNumbers() const
{
    LIST_PROPERTY_FROM_DETAIL_FIELD(QContactPhoneNumber, number);
}

#define SET_PROPERTY_FIELD_FROM_LIST(detailType, fieldNameGet, fieldNameSet, newValueList) \
    const QList<detailType> &oldDetailList = mContact.details<detailType>(); \
    \
    if (oldDetailList.count() != newValueList.count()) { \
        bool removeAndReadd = true; \
        if (oldDetailList.count() < newValueList.count()) { \
            /* Check to see if existing details were modified at all */ \
            bool modification = false; \
            for (int i = 0; i < oldDetailList.count(); ++i) { \
                if (oldDetailList.at(i).fieldNameGet() != newValueList.at(i)) { \
                    modification = true; \
                    break; \
                } \
            } \
    \
            if (!modification) { \
                /* If the only changes are new additions, just add them. */ \
                for (int i = oldDetailList.count(); i < newValueList.count(); ++i) { \
                    detailType detail; \
                    detail.fieldNameSet(newValueList.at(i)); \
                    mContact.saveDetail(&detail); \
                } \
                removeAndReadd = false; \
            } else { \
                removeAndReadd = true; \
            } \
        } \
    \
        if (removeAndReadd) { \
            foreach (detailType detail, oldDetailList) \
                mContact.removeDetail(&detail); \
    \
            foreach (const QString &value, newValueList) { \
                detailType detail; \
                detail.fieldNameSet(value); \
                mContact.saveDetail(&detail); \
            } \
        } \
    } else { \
        /* assign new numbers to the existing details. */ \
        for (int i = 0; i != newValueList.count(); ++i) { \
            detailType detail = oldDetailList.at(i); \
            detail.fieldNameSet(newValueList.at(i)); \
            mContact.saveDetail(&detail); \
        } \
    }

void SeasidePerson::setPhoneNumbers(const QStringList &phoneNumbers)
{
    SET_PROPERTY_FIELD_FROM_LIST(QContactPhoneNumber, number, setNumber, phoneNumbers)
    emit phoneNumbersChanged();
}

bool SeasidePerson::hasPhoneType(const QString &context, const QString &subType) const
{
    const QList<QContactPhoneNumber> &numbers = mContact.details<QContactPhoneNumber>();
    for (int i=0; i<numbers.count(); i++) {
        if (numbers[i].contexts().contains(context) && numbers[i].subTypes().contains(subType)) {
            return true;
        }
    }
    return false;
}

QList<int> SeasidePerson::phoneNumberTypes() const
{
    const QList<QContactPhoneNumber> &numbers = mContact.details<QContactPhoneNumber>();
    QList<int> types;
    types.reserve((numbers.length()));

    foreach(const QContactPhoneNumber &number, numbers) {
        if (number.contexts().contains(QContactDetail::ContextHome)
            && number.subTypes().contains(QContactPhoneNumber::SubTypeLandline)) {
            types.push_back(SeasidePerson::PhoneHomeType);
        } else if (number.contexts().contains(QContactDetail::ContextWork)
            && number.subTypes().contains(QContactPhoneNumber::SubTypeLandline)) {
            types.push_back(SeasidePerson::PhoneWorkType);
        } else if (number.contexts().contains(QContactDetail::ContextHome)
            && number.subTypes().contains(QContactPhoneNumber::SubTypeMobile)) {
            types.push_back(SeasidePerson::PhoneMobileType);
        } else if (number.contexts().contains(QContactDetail::ContextHome)
            && number.subTypes().contains(QContactPhoneNumber::SubTypeFax)) {
            types.push_back(SeasidePerson::PhoneFaxType);
        } else if (number.contexts().contains(QContactDetail::ContextHome)
            && number.subTypes().contains(QContactPhoneNumber::SubTypePager)) {
            types.push_back(SeasidePerson::PhonePagerType);
        } else {
            qWarning() << "Warning: Could not get phone type for '" << number.contexts() << "'";
        }
    }

    return types;
}

void SeasidePerson::setPhoneNumberType(int which, SeasidePerson::DetailType type)
{
    const QList<QContactPhoneNumber> &numbers = mContact.details<QContactPhoneNumber>();
    if (which >= numbers.length()) {
        qWarning() << "Unable to set type for phone number: invalid index specified. Aborting.";
        return;
    }

    QContactPhoneNumber number = numbers.at(which);
    if (type == SeasidePerson::PhoneHomeType) {
        number.setSubTypes(QContactPhoneNumber::SubTypeLandline);
        number.setContexts(QContactDetail::ContextHome);
    }  else if (type == SeasidePerson::PhoneWorkType) {
        number.setSubTypes(QContactPhoneNumber::SubTypeLandline);
        number.setContexts(QContactDetail::ContextWork);
    } else if (type == SeasidePerson::PhoneMobileType) {
        number.setSubTypes(QContactPhoneNumber::SubTypeMobile);
        number.setContexts(QContactDetail::ContextHome);
    } else if (type == SeasidePerson::PhoneFaxType) {
        number.setSubTypes(QContactPhoneNumber::SubTypeFax);
        number.setContexts(QContactDetail::ContextHome);
    } else if (type == SeasidePerson::PhonePagerType) {
        number.setSubTypes(QContactPhoneNumber::SubTypePager);
        number.setContexts(QContactDetail::ContextHome);
    } else {
        qWarning() << "Warning: Could not save phone type '" << type << "'";
    }

    mContact.saveDetail(&number);
    emit phoneNumberTypesChanged();
}

QStringList SeasidePerson::emailAddresses() const
{
    LIST_PROPERTY_FROM_DETAIL_FIELD(QContactEmailAddress, emailAddress);
}

void SeasidePerson::setEmailAddresses(const QStringList &emailAddresses)
{
    SET_PROPERTY_FIELD_FROM_LIST(QContactEmailAddress, emailAddress, setEmailAddress, emailAddresses)
    emit emailAddressesChanged();
}

bool SeasidePerson::hasEmailType(const QString &context) const
{
    const QList<QContactEmailAddress> &emails = mContact.details<QContactEmailAddress>();
    for (int i=0; i<emails.count(); i++) {
        if (emails[i].contexts().contains(context)) {
            return true;
        }
    }
    return false;
}

QList<int> SeasidePerson::emailAddressTypes() const
{
    const QList<QContactEmailAddress> &emails = mContact.details<QContactEmailAddress>();
    QList<int> types;
    types.reserve((emails.length()));

    foreach(const QContactEmailAddress &email, emails) {
        if (email.contexts().contains(QContactDetail::ContextHome)) {
            types.push_back(SeasidePerson::EmailHomeType);
        } else if (email.contexts().contains(QContactDetail::ContextWork)) {
            types.push_back(SeasidePerson::EmailWorkType);
        } else if (email.contexts().contains(QContactDetail::ContextOther)) {
            types.push_back(SeasidePerson::EmailOtherType);
        } else {
            qWarning() << "Warning: Could not get email type '" << email.contexts() << "'";
        }
    }

    return types;
}

void SeasidePerson::setEmailAddressType(int which, SeasidePerson::DetailType type)
{
    const QList<QContactEmailAddress> &emails = mContact.details<QContactEmailAddress>();

    if (which >= emails.length()) {
        qWarning() << "Unable to set type for email address: invalid index specified. Aborting.";
        return;
    }

    QContactEmailAddress email = emails.at(which);
    if (type == SeasidePerson::EmailHomeType) {
        email.setContexts(QContactDetail::ContextHome);
    }  else if (type == SeasidePerson::EmailWorkType) {
        email.setContexts(QContactDetail::ContextWork);
    } else if (type == SeasidePerson::EmailOtherType) {
        email.setContexts(QContactDetail::ContextOther);
    } else {
        qWarning() << "Warning: Could not save email type '" << type << "'";
    }

    mContact.saveDetail(&email);
    emit emailAddressTypesChanged();
}

// Fields are separated by \n characters
QStringList SeasidePerson::addresses() const
{
    QStringList retn;
    const QList<QContactAddress> &addresses = mContact.details<QContactAddress>();
    foreach (const QContactAddress &address, addresses) {
        QString currAddressStr;
        currAddressStr.append(address.street());
        currAddressStr.append("\n");
        currAddressStr.append(address.locality());
        currAddressStr.append("\n");
        currAddressStr.append(address.region());
        currAddressStr.append("\n");
        currAddressStr.append(address.postcode());
        currAddressStr.append("\n");
        currAddressStr.append(address.country());
        currAddressStr.append("\n");
        currAddressStr.append(address.postOfficeBox());
        retn.append(currAddressStr);
    }
    return retn;
}

void SeasidePerson::setAddresses(const QStringList &addresses)
{
    QList<QStringList> splitStrings;
    foreach (const QString &currAddressStr, addresses) {
        QStringList split = currAddressStr.split("\n");
        if (split.count() != 6) {
            qWarning() << "Warning: Could not save addresses - invalid format for address:" << currAddressStr;
            return;
        } else {
            splitStrings.append(split);
        }
    }

    const QList<QContactAddress> &oldDetailList = mContact.details<QContactAddress>();
    if (oldDetailList.count() != splitStrings.count()) {
        /* remove all current details, recreate new ones */
        foreach (QContactAddress oldAddress, oldDetailList)
            mContact.removeDetail(&oldAddress);
        foreach (const QStringList &split, splitStrings) {
            QContactAddress newAddress;
            newAddress.setStreet(split.at(0));
            newAddress.setLocality(split.at(1));
            newAddress.setRegion(split.at(2));
            newAddress.setPostcode(split.at(3));
            newAddress.setCountry(split.at(4));
            newAddress.setPostOfficeBox(split.at(5));
            mContact.saveDetail(&newAddress);
        }
    } else {
        /* overwrite existing details */
        for (int i = 0; i < splitStrings.count(); ++i) {
            const QStringList &split = splitStrings.at(i);
            QContactAddress oldAddress = oldDetailList.at(i);
            oldAddress.setStreet(split.at(0));
            oldAddress.setLocality(split.at(1));
            oldAddress.setRegion(split.at(2));
            oldAddress.setPostcode(split.at(3));
            oldAddress.setCountry(split.at(4));
            oldAddress.setPostOfficeBox(split.at(5));
            mContact.saveDetail(&oldAddress);
        }
    }

    emit addressesChanged();
}

bool SeasidePerson::hasAddressType(const QString &context) const
{
    const QList<QContactAddress> &addresses = mContact.details<QContactAddress>();
    for (int i=0; i<addresses.count(); i++) {
        if (addresses[i].contexts().contains(context)) {
            return true;
        }
    }
    return false;
}

QList<int> SeasidePerson::addressTypes() const
{
    const QList<QContactAddress> &addresses = mContact.details<QContactAddress>();
    QList<int> types;
    types.reserve((addresses.length()));

    foreach(const QContactAddress &address, addresses) {
        if (address.contexts().contains(QContactDetail::ContextHome)) {
            types.push_back(SeasidePerson::AddressHomeType);
        } else if (address.contexts().contains(QContactDetail::ContextWork)) {
            types.push_back(SeasidePerson::AddressWorkType);
        } else if (address.contexts().contains(QContactDetail::ContextOther)) {
            types.push_back(SeasidePerson::AddressOtherType);
        } else {
            qWarning() << "Warning: Could not get address type '" << address.contexts() << "'";
        }
    }

    return types;
}

void SeasidePerson::setAddressType(int which, SeasidePerson::DetailType type)
{
    const QList<QContactAddress> &addresses = mContact.details<QContactAddress>();

    if (which >= addresses.length()) {
        qWarning() << "Unable to set type for address: invalid index specified. Aborting.";
        return;
    }

    QContactAddress address = addresses.at(which);
    if (type == SeasidePerson::AddressHomeType) {
        address.setContexts(QContactDetail::ContextHome);
    }  else if (type == SeasidePerson::AddressWorkType) {
        address.setContexts(QContactDetail::ContextWork);
    } else if (type == SeasidePerson::AddressOtherType) {
        address.setContexts(QContactDetail::ContextOther);
    } else {
        qWarning() << "Warning: Could not save address type '" << type << "'";
    }

    mContact.saveDetail(&address);
    emit addressTypesChanged();
}

QStringList SeasidePerson::websites() const
{
    LIST_PROPERTY_FROM_DETAIL_FIELD(QContactUrl, url);
}

void SeasidePerson::setWebsites(const QStringList &websites)
{
    SET_PROPERTY_FIELD_FROM_LIST(QContactUrl, url, setUrl, websites)
    emit websitesChanged();
}

bool SeasidePerson::hasWebsiteType(const QString &context) const
{
    const QList<QContactUrl> &websites = mContact.details<QContactUrl>();
    for (int i=0; i<websites.count(); i++) {
        if (websites[i].contexts().contains(context)) {
            return true;
        }
    }
    return false;
}

QList<int> SeasidePerson::websiteTypes() const
{
    const QList<QContactUrl> &urls = mContact.details<QContactUrl>();
    QList<int> types;
    types.reserve((urls.length()));

    foreach(const QContactUrl &url, urls) {
        if (url.contexts().contains(QContactDetail::ContextHome)) {
            types.push_back(SeasidePerson::WebsiteHomeType);
        } else if (url.contexts().contains(QContactDetail::ContextWork)) {
            types.push_back(SeasidePerson::WebsiteWorkType);
        } else if (url.contexts().contains(QContactDetail::ContextOther)) {
            types.push_back(SeasidePerson::WebsiteOtherType);
        } else {
            qWarning() << "Warning: Could not get website type '" << url.contexts() << "'";
        }
    }

    return types;
}

void SeasidePerson::setWebsiteType(int which, SeasidePerson::DetailType type)
{
    const QList<QContactUrl> &urls = mContact.details<QContactUrl>();

    if (which >= urls.length()) {
        qWarning() << "Unable to set type for website: invalid index specified. Aborting.";
        return;
    }

    QContactUrl url = urls.at(which);
    if (type == SeasidePerson::WebsiteHomeType) {
        url.setContexts(QContactDetail::ContextHome);
    }  else if (type == SeasidePerson::WebsiteWorkType) {
        url.setContexts(QContactDetail::ContextWork);
    } else if (type == SeasidePerson::WebsiteOtherType) {
        url.setContexts(QContactDetail::ContextOther);
    } else {
        qWarning() << "Warning: Could not save website type '" << type << "'";
    }

    mContact.saveDetail(&url);
    emit websiteTypesChanged();
}

QDateTime SeasidePerson::birthday() const
{
    return mContact.detail<QContactBirthday>().dateTime();
}

void SeasidePerson::setBirthday(const QDateTime &bd)
{
    QContactBirthday birthday = mContact.detail<QContactBirthday>();
    birthday.setDateTime(bd);
    mContact.saveDetail(&birthday);
    emit birthdayChanged();
}

QDateTime SeasidePerson::anniversary() const
{
    return mContact.detail<QContactAnniversary>().originalDateTime();
}

void SeasidePerson::setAnniversary(const QDateTime &av)
{
    QContactAnniversary anniv = mContact.detail<QContactAnniversary>();
    anniv.setOriginalDateTime(av);
    mContact.saveDetail(&anniv);
    emit anniversaryChanged();
}

// TODO: merge with LIST_PROPERTY_FROM_DETAIL_FIELD
#define LIST_PROPERTY_FROM_FIELD_NAME(detailType, fieldName) \
    QStringList list; \
    \
    foreach (const detailType &detail, mContact.details<detailType>()) { \
        if (detail.hasValue(fieldName)) \
            list << detail.value(fieldName); \
    } \
    return list;

QStringList SeasidePerson::accountUris() const
{
    LIST_PROPERTY_FROM_FIELD_NAME(QContactOnlineAccount, QContactOnlineAccount::FieldAccountUri)
}

QStringList SeasidePerson::accountPaths() const
{
    LIST_PROPERTY_FROM_FIELD_NAME(QContactOnlineAccount, "AccountPath") // QContactOnlineAccount__FieldAccountPath
}

QContact SeasidePerson::contact() const
{
    return mContact;
}

void SeasidePerson::setContact(const QContact &contact)
{
    QContact oldContact = mContact;
    mContact = contact;

    QContactName oldName = oldContact.detail<QContactName>();
    QContactName newName = mContact.detail<QContactName>();
    if (oldName.firstName() != newName.firstName())
        emit firstNameChanged();
    if (oldName.lastName() != newName.lastName())
        emit lastNameChanged();
    if (oldName.middleName() != newName.middleName())
        emit middleNameChanged();
    if (oldName.prefix() != newName.prefix())
        emit titleChanged();

    recalculateDisplayLabel();

    if (oldContact.detail<QContactOrganization>().name() != mContact.detail<QContactOrganization>().name())
        emit companyNameChanged();

    if (oldContact.detail<QContactNickname>().nickname() != mContact.detail<QContactNickname>().nickname())
        emit nicknameChanged();

    if (oldContact.details<QContactPhoneNumber>() != mContact.details<QContactPhoneNumber>()) {
        emit phoneNumbersChanged();
        emit phoneNumberTypesChanged();
    }

    if (oldContact.details<QContactEmailAddress>() != mContact.details<QContactEmailAddress>()) {
        emit emailAddressesChanged();
        emit emailAddressTypesChanged();
    }

    if (oldContact.details<QContactAddress>() != mContact.details<QContactAddress>()) {
        emit addressesChanged();
        emit addressTypesChanged();
    }

    if (oldContact.details<QContactUrl>() != mContact.details<QContactUrl>()) {
        emit websitesChanged();
        emit websiteTypesChanged();
    }

    if (oldContact.detail<QContactBirthday>().dateTime() != mContact.detail<QContactBirthday>().dateTime())
        emit birthdayChanged();

    if (oldContact.detail<QContactAnniversary>().originalDateTime() != mContact.detail<QContactAnniversary>().originalDateTime())
        emit anniversaryChanged();

    QContactFavorite oldFavorite = oldContact.detail<QContactFavorite>();
    QContactFavorite newFavorite = mContact.detail<QContactFavorite>();
    if (oldFavorite.isFavorite() != newFavorite.isFavorite())
        emit favoriteChanged();

    QContactAvatar oldAvatar = oldContact.detail<QContactAvatar>();
    QContactAvatar newAvatar = mContact.detail<QContactAvatar>();
    if (oldAvatar.imageUrl() != newAvatar.imageUrl())
        emit avatarPathChanged();

    // TODO: differencing of list type details
    emit accountUrisChanged();
    emit accountPathsChanged();
}

