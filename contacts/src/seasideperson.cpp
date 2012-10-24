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
#include <QContactName>
#include <QContactFavorite>
#include <QContactPhoneNumber>
#include <QContactEmailAddress>
#include <QContactOnlineAccount>
#include <QContactOrganization>

#include "seasideperson.h"

/**
 * String identifying the contact detail from the UI.
 */
const char *SeasidePerson::SP_TYPE_PHONE_HOME   = "phone_home";
const char *SeasidePerson::SP_TYPE_PHONE_WORK   = "phone_work";
const char *SeasidePerson::SP_TYPE_PHONE_MOBILE = "phone_mobile";
const char *SeasidePerson::SP_TYPE_PHONE_FAX    = "phone_fax";
const char *SeasidePerson::SP_TYPE_PHONE_PAGER  = "phone_pager";

const char *SeasidePerson::SP_TYPE_EMAIL_HOME   = "email_personal";
const char *SeasidePerson::SP_TYPE_EMAIL_WORK   = "email_work";
const char *SeasidePerson::SP_TYPE_EMAIL_OTHER  = "email_other";


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

// small helper to avoid inconvenience
static QString generateDisplayLabel(QContact mContact)
{
    //REVISIT: Move this or parts of this to localeutils.cpp
    QString displayLabel;
    QContactName name = mContact.detail<QContactName>();
    QString nameStr1 = name.firstName();
    QString nameStr2 = name.lastName();

    if (!nameStr1.isNull())
        displayLabel.append(nameStr1);

    if (!nameStr2.isNull()) {
        if (!displayLabel.isEmpty())
            displayLabel.append(" ");
        displayLabel.append(nameStr2);
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
    QString oldDisplayLabel = displayLabel();
    QString newDisplayLabel = generateDisplayLabel(mContact);

    // TODO: would be lovely if mobility would let us store this somehow
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

// this could probably be optimised, but it's easiest: we just remove
// all the old phone number details, and add new ones
#define SET_PROPERTY_FIELD_FROM_LIST(detailType, fieldNameGet, fieldNameSet, newValueList) \
    const QList<detailType> &oldDetailList = mContact.details<detailType>(); \
    \
    if (oldDetailList.count() != newValueList.count()) { \
        foreach (detailType detail, oldDetailList) \
            mContact.removeDetail(&detail); \
    \
        foreach (const QString &value, newValueList) { \
            detailType detail; \
            detail.fieldNameSet(value); \
            mContact.saveDetail(&detail); \
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

QStringList SeasidePerson::phoneNumberTypes() const
{
    const QList<QContactPhoneNumber> &numbers = mContact.details<QContactPhoneNumber>();
    QStringList types;
    types.reserve((numbers.length()));

    foreach(const QContactPhoneNumber &number, numbers) {
        if (number.contexts().contains(QContactDetail::ContextHome)
            && number.subTypes().contains(QContactPhoneNumber::SubTypeLandline)) {
            types.push_back(SP_TYPE_PHONE_HOME);
        } else if (number.contexts().contains(QContactDetail::ContextWork)
            && number.subTypes().contains(QContactPhoneNumber::SubTypeLandline)) {
            types.push_back(SP_TYPE_PHONE_WORK);
        } else if (number.contexts().contains(QContactDetail::ContextHome)
            && number.subTypes().contains(QContactPhoneNumber::SubTypeMobile)) {
            types.push_back(SP_TYPE_PHONE_MOBILE);
        } else if (number.contexts().contains(QContactDetail::ContextHome)
            && number.subTypes().contains(QContactPhoneNumber::SubTypeFax)) {
            types.push_back(SP_TYPE_PHONE_FAX);
        } else if (number.contexts().contains(QContactDetail::ContextHome)
            && number.subTypes().contains(QContactPhoneNumber::SubTypePager)) {
            types.push_back(SP_TYPE_PHONE_PAGER);
        } else {
            qWarning() << "Warning: Could not get phone context '" << number.contexts() << "'";
        }
    }

    return types;
}

void SeasidePerson::setPhoneNumberTypes(const QStringList &phoneNumberTypes)
{
    const QList<QContactPhoneNumber> &numbers = mContact.details<QContactPhoneNumber>();
    int i=0;
    QContactPhoneNumber number;

    if (phoneNumberTypes.length() != numbers.length()) {
        qWarning() << "Number of phone numbers does not match the number of contexts being stored. Aborting.";
        return;
    }

    foreach(const QString &type, phoneNumberTypes) {
        number = numbers.at(i);

        if (type == SeasidePerson::SP_TYPE_PHONE_HOME) {
            number.setSubTypes(QContactPhoneNumber::SubTypeLandline);
            number.setContexts(QContactDetail::ContextHome);
        }  else if (type == SeasidePerson::SP_TYPE_PHONE_WORK) {
            number.setSubTypes(QContactPhoneNumber::SubTypeLandline);
            number.setContexts(QContactDetail::ContextWork);
        } else if (type == SeasidePerson::SP_TYPE_PHONE_MOBILE) {
            number.setSubTypes(QContactPhoneNumber::SubTypeMobile);
            number.setContexts(QContactDetail::ContextHome);
        } else if (type == SeasidePerson::SP_TYPE_PHONE_FAX) {
            number.setSubTypes(QContactPhoneNumber::SubTypeFax);
            number.setContexts(QContactDetail::ContextHome);
        } else if (type == SeasidePerson::SP_TYPE_PHONE_PAGER) {
            number.setSubTypes(QContactPhoneNumber::SubTypePager);
            number.setContexts(QContactDetail::ContextHome);
        } else {
            qWarning() << "Warning: Could not save phone type '" << type << "'";
        }

        mContact.saveDetail(&number);
        i++;
    }

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

QStringList SeasidePerson::emailAddressTypes() const
{
    const QList<QContactEmailAddress> &emails = mContact.details<QContactEmailAddress>();
    QStringList types;
    types.reserve((emails.length()));

    foreach(const QContactEmailAddress &email, emails) {
        if (email.contexts().contains(QContactDetail::ContextHome)) {
            types.push_back(SP_TYPE_EMAIL_HOME);
        } else if (email.contexts().contains(QContactDetail::ContextWork)) {
            types.push_back(SP_TYPE_EMAIL_WORK);
        } else if (email.contexts().contains(QContactDetail::ContextOther)) {
            types.push_back(SP_TYPE_EMAIL_OTHER);
        } else {
            qWarning() << "Warning: Could not get email context '" << email.contexts() << "'";
        }
    }

    return types;
}

void SeasidePerson::setEmailAddressTypes(const QStringList &emailAddressTypes)
{
    const QList<QContactEmailAddress> &emails = mContact.details<QContactEmailAddress>();
    int i=0;
    QContactEmailAddress email;

    if (emailAddressTypes.length() != emails.length()) {
        qWarning() << "Number of email addresses does not match the number of contexts being stored. Aborting.";
        return;
    }

    foreach(const QString &type, emailAddressTypes) {
        email = emails.at(i);

        if (type == SP_TYPE_EMAIL_HOME) {
            email.setContexts(QContactDetail::ContextHome);
        }  else if (type == SP_TYPE_EMAIL_WORK) {
            email.setContexts(QContactDetail::ContextWork);
        } else if (type == SP_TYPE_EMAIL_OTHER) {
            email.setContexts(QContactDetail::ContextOther);
        } else {
            qWarning() << "Warning: Could not save email type '" << type << "'";
        }

        mContact.saveDetail(&email);
        i++;
    }

    emit emailAddressTypesChanged();
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

    QContactOrganization oldCompany = oldContact.detail<QContactOrganization>();
    QContactOrganization newCompany = mContact.detail<QContactOrganization>();

    if (oldCompany.name() != newCompany.name())
        emit companyNameChanged();

    QContactFavorite oldFavorite = oldContact.detail<QContactFavorite>();
    QContactFavorite newFavorite = mContact.detail<QContactFavorite>();

    if (oldFavorite.isFavorite() != newFavorite.isFavorite())
        emit favoriteChanged();

    QContactAvatar oldAvatar = oldContact.detail<QContactAvatar>();
    QContactAvatar newAvatar = mContact.detail<QContactAvatar>();

    if (oldAvatar.imageUrl() != newAvatar.imageUrl())
        emit avatarPathChanged();

    // TODO: differencing of list type details
    emit phoneNumbersChanged();
    emit emailAddressesChanged();
    emit accountUrisChanged();
    emit accountPathsChanged();

    recalculateDisplayLabel();
}

