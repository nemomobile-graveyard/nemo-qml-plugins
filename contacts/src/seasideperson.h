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

#ifndef SEASIDEPERSON_H
#define SEASIDEPERSON_H

// Qt
#include <QObject>
#include <QUrl>
#include <qdeclarative.h>

// Mobility
#include <QContact>
#include <QContactPresence>

// Seaside
#include "seasideproxymodel.h"

QTM_USE_NAMESPACE

Q_DECLARE_METATYPE(QContact)

class SeasidePerson;

class SeasidePersonAttached : public QObject
{
    Q_OBJECT
    Q_PROPERTY(SeasidePerson *selfPerson READ selfPerson NOTIFY selfPersonChanged)

public:
    SeasidePersonAttached(QObject *parent);

    SeasidePerson *selfPerson() const;

signals:
    void selfPersonChanged();
};

class SeasidePerson : public QObject
{
    Q_OBJECT
    Q_ENUMS(DetailType)
    Q_ENUMS(PresenceState)

public:
    /**
     * Identifiers of contact details for the UI.
     */
    enum DetailType {
        // Name
        FirstNameType,
        LastNameType,
        MiddleNameType,
        CompanyType,
        NickType,
        TitleType,
        // Phone
        PhoneHomeType,
        PhoneWorkType,
        PhoneMobileType,
        PhoneFaxType,
        PhonePagerType,
        // Email
        EmailHomeType,
        EmailWorkType,
        EmailOtherType,
        // Address
        AddressHomeType,
        AddressWorkType,
        AddressOtherType,
        AddressStreetType,
        AddressLocalityType,
        AddressRegionType,
        AddressPostcodeType,
        AddressCountryType,
        AddressPOBoxType,
        // Website
        WebsiteHomeType,
        WebsiteWorkType,
        WebsiteOtherType,
        // Dates
        BirthdayType,
        AnniversaryType,
        // Presence information
        GlobalPresenceStateType,
        AccountProvidersType,
        PresenceAccountProvidersType = AccountProvidersType,  // To be removed
        AccountIconPathsType,
        PresenceStatesType,
        PresenceMessagesType,
    };

    enum PresenceState {
        PresenceUnknown = QContactPresence::PresenceUnknown,
        PresenceAvailable = QContactPresence::PresenceAvailable,
        PresenceHidden = QContactPresence::PresenceHidden,
        PresenceBusy = QContactPresence::PresenceBusy,
        PresenceAway = QContactPresence::PresenceAway,
        PresenceExtendedAway = QContactPresence::PresenceExtendedAway,
        PresenceOffline = QContactPresence::PresenceOffline
    };

    explicit SeasidePerson(QObject *parent = 0);
    ~SeasidePerson();

    Q_PROPERTY(int id READ id NOTIFY contactChanged)
    int id() const;

    Q_PROPERTY(bool complete READ isComplete NOTIFY completeChanged)
    bool isComplete() const;
    void setComplete(bool complete);

    Q_PROPERTY(QString firstName READ firstName WRITE setFirstName NOTIFY firstNameChanged)
    QString firstName() const;
    void setFirstName(const QString &name);

    Q_PROPERTY(QString lastName READ lastName WRITE setLastName NOTIFY lastNameChanged)
    QString lastName() const;
    void setLastName(const QString &name);

    Q_PROPERTY(QString middleName READ middleName WRITE setMiddleName NOTIFY middleNameChanged)
    QString middleName() const;
    void setMiddleName(const QString &name);

    Q_PROPERTY(QString sectionBucket READ sectionBucket NOTIFY displayLabelChanged)
    QString sectionBucket();

    Q_PROPERTY(QString displayLabel READ displayLabel NOTIFY displayLabelChanged)
    QString displayLabel();

    Q_PROPERTY(QString companyName READ companyName WRITE setCompanyName NOTIFY companyNameChanged)
    QString companyName() const;
    void setCompanyName(const QString &name);

    Q_PROPERTY(QString nickname READ nickname WRITE setNickname NOTIFY nicknameChanged)
    QString nickname() const;
    void setNickname(const QString &name);

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    QString title() const;
    void setTitle(const QString &name);

    Q_PROPERTY(bool favorite READ favorite WRITE setFavorite NOTIFY favoriteChanged)
    bool favorite() const;
    void setFavorite(bool favorite);

    Q_PROPERTY(QUrl avatarPath READ avatarPath WRITE setAvatarPath NOTIFY avatarPathChanged)
    QUrl avatarPath() const;
    void setAvatarPath(QUrl avatarPath);

    Q_PROPERTY(QStringList phoneNumbers READ phoneNumbers WRITE setPhoneNumbers NOTIFY phoneNumbersChanged)
    QStringList phoneNumbers() const;
    void setPhoneNumbers(const QStringList &phoneNumbers);

    Q_PROPERTY(QList<int> phoneNumberTypes READ phoneNumberTypes NOTIFY phoneNumberTypesChanged)
    QList<int> phoneNumberTypes() const;
    Q_INVOKABLE void setPhoneNumberType(int which, DetailType type);

    Q_PROPERTY(QStringList emailAddresses READ emailAddresses WRITE setEmailAddresses NOTIFY emailAddressesChanged)
    QStringList emailAddresses() const;
    void setEmailAddresses(const QStringList &emailAddresses);

    Q_PROPERTY(QList<int> emailAddressTypes READ emailAddressTypes NOTIFY emailAddressTypesChanged)
    QList<int> emailAddressTypes() const;
    Q_INVOKABLE void setEmailAddressType(int which, DetailType type);

    Q_PROPERTY(QStringList addresses READ addresses WRITE setAddresses NOTIFY addressesChanged)
    QStringList addresses() const;
    void setAddresses(const QStringList &addresses);

    Q_PROPERTY(QList<int> addressTypes READ addressTypes NOTIFY addressTypesChanged)
    QList<int> addressTypes() const;
    Q_INVOKABLE void setAddressType(int which, DetailType type);

    Q_PROPERTY(QStringList websites READ websites WRITE setWebsites NOTIFY websitesChanged)
    QStringList websites() const;
    void setWebsites(const QStringList &sites);

    Q_PROPERTY(QList<int> websiteTypes READ websiteTypes NOTIFY websiteTypesChanged)
    QList<int> websiteTypes() const;
    Q_INVOKABLE void setWebsiteType(int which, DetailType type);

    Q_PROPERTY(QDateTime birthday READ birthday WRITE setBirthday NOTIFY birthdayChanged RESET resetBirthday)
    QDateTime birthday() const;
    void setBirthday(const QDateTime &bd);
    void resetBirthday();

    Q_PROPERTY(QDateTime anniversary READ anniversary WRITE setAnniversary NOTIFY anniversaryChanged RESET resetAnniversary)
    QDateTime anniversary() const;
    void setAnniversary(const QDateTime &av);
    void resetAnniversary();

    Q_PROPERTY(PresenceState globalPresenceState READ globalPresenceState NOTIFY globalPresenceStateChanged)
    PresenceState globalPresenceState() const;

    // To be removed:
    Q_PROPERTY(QStringList presenceAccountProviders READ presenceAccountProviders NOTIFY presenceAccountProvidersChanged)
    QStringList presenceAccountProviders() const;

    Q_PROPERTY(QList<int> presenceStates READ presenceStates NOTIFY presenceStatesChanged)
    QList<int> presenceStates() const;

    Q_PROPERTY(QStringList presenceMessages READ presenceMessages NOTIFY presenceMessagesChanged)
    QStringList presenceMessages() const;

    Q_PROPERTY(QStringList accountUris READ accountUris NOTIFY accountUrisChanged)
    QStringList accountUris() const;

    Q_PROPERTY(QStringList accountPaths READ accountPaths NOTIFY accountPathsChanged)
    QStringList accountPaths() const;

    Q_PROPERTY(QStringList accountProviders READ accountProviders NOTIFY accountProvidersChanged)
    QStringList accountProviders() const;

    Q_PROPERTY(QStringList accountIconPaths READ accountIconPaths NOTIFY accountIconPathsChanged)
    QStringList accountIconPaths() const;

    Q_INVOKABLE void addAccount(const QString &path, const QString &uri, const QString &provider,
                                const QString &iconPath = QString());

    QContact contact() const;
    void setContact(const QContact &contact);

    Q_INVOKABLE QVariant contactData() const { return QVariant::fromValue(contact()); }
    Q_INVOKABLE void setContactData(const QVariant &data) { setContact(data.value<QContact>()); }

    Q_INVOKABLE QString vCard() const;

    static QString generateDisplayLabel(
                const QContact &mContact,
                SeasideProxyModel::DisplayLabelOrder order = SeasideProxyModel::FirstNameFirst);
    static QString generateDisplayLabelFromNonNameDetails(const QContact &mContact);

    static SeasidePersonAttached *qmlAttachedProperties(QObject *object);

signals:
    void contactChanged();
    void contactRemoved();
    void completeChanged();
    void firstNameChanged();
    void lastNameChanged();
    void middleNameChanged();
    void displayLabelChanged();
    void companyNameChanged();
    void nicknameChanged();
    void titleChanged();
    void favoriteChanged();
    void avatarPathChanged();
    void phoneNumbersChanged();
    void phoneNumberTypesChanged();
    void emailAddressesChanged();
    void emailAddressTypesChanged();
    void addressesChanged();
    void addressTypesChanged();
    void websitesChanged();
    void websiteTypesChanged();
    void birthdayChanged();
    void anniversaryChanged();
    void globalPresenceStateChanged();
    void presenceAccountProvidersChanged();
    void presenceStatesChanged();
    void presenceMessagesChanged();
    void accountUrisChanged();
    void accountPathsChanged();
    void accountProvidersChanged();
    void accountIconPathsChanged();

public slots:
    void recalculateDisplayLabel(SeasideProxyModel::DisplayLabelOrder order = SeasideProxyModel::FirstNameFirst);

private:
    // TODO: private class
    explicit SeasidePerson(const QContact &contact, QObject *parent = 0);
    QContact mContact;
    QString mDisplayLabel;
    bool mComplete;

    friend class tst_SeasidePerson;
    friend class SeasidePeopleModelPriv;
};

QML_DECLARE_TYPEINFO(SeasidePerson, QML_HAS_ATTACHED_PROPERTIES);

QML_DECLARE_TYPE(SeasidePerson);

#endif // SEASIDEPERSON_H
