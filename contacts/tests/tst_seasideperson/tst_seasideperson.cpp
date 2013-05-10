/*
 * Copyright (C) 2012 Jolla Mobile <robin.burchell@jollamobile.com>
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

#include <QObject>
#include <QtTest>

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

QTM_USE_NAMESPACE

class tst_SeasidePerson : public QObject
{
    Q_OBJECT

private slots:
    void firstName();
    void lastName();
    void middleName();
    void displayLabel();
    void sectionBucket();
    void companyName();
    void nickname();
    void title();
    void favorite();
    void avatarPath();
    void phoneNumbers();
    void phoneTypes();
    void emailAddresses();
    void emailTypes();
    void websites();
    void websiteTypes();
    void birthday();
    void anniversary();
    void address();
    void globalPresenceState();
    void complete();
    void marshalling();
    void setContact();
    void vcard();
};

void tst_SeasidePerson::firstName()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);
    QCOMPARE(person->firstName(), QString());
    QSignalSpy spy(person.data(), SIGNAL(firstNameChanged()));
    person->setFirstName("Test");
    QCOMPARE(spy.count(), 1);
    QCOMPARE(person->firstName(), QString::fromLatin1("Test"));
    QCOMPARE(person->property("firstName").toString(), person->firstName());
}

void tst_SeasidePerson::lastName()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);
    QCOMPARE(person->lastName(), QString());
    QSignalSpy spy(person.data(), SIGNAL(lastNameChanged()));
    person->setLastName("Test");
    QCOMPARE(spy.count(), 1);
    QCOMPARE(person->lastName(), QString::fromLatin1("Test"));
    QCOMPARE(person->property("lastName").toString(), person->lastName());
}

void tst_SeasidePerson::middleName()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);
    QCOMPARE(person->middleName(), QString());
    QSignalSpy spy(person.data(), SIGNAL(middleNameChanged()));
    person->setMiddleName("Test");
    QCOMPARE(spy.count(), 1);
    QCOMPARE(person->middleName(), QString::fromLatin1("Test"));
    QCOMPARE(person->property("middleName").toString(), person->middleName());
}

void tst_SeasidePerson::displayLabel()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);
    QSignalSpy spy(person.data(), SIGNAL(displayLabelChanged()));
    person->setFirstName("Test");
    person->setLastName("Last");
    QCOMPARE(spy.count(), 2); // TODO: it would be nicer if this would only emit once per event loop
    QCOMPARE(person->firstName(), QString::fromLatin1("Test"));
    QCOMPARE(person->lastName(), QString::fromLatin1("Last"));
    QCOMPARE(person->displayLabel(), QString::fromLatin1("Test Last"));

    // TODO: test additional cases for displayLabel:
    // - email/IM id
    // - company
    // - other fallbacks?
    // - "(unnamed)"
}

void tst_SeasidePerson::sectionBucket()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);
    QSignalSpy spy(person.data(), SIGNAL(displayLabelChanged()));
    QCOMPARE(person->displayLabel(), QString("(Unnamed)"));
    QCOMPARE(spy.count(), 1);

    // XXX This should be in a section bucket of "unnamed" contacts?
//    QCOMPARE(person->sectionBucket(), QString());

    // set first
    person->setLastName("Test");
    QCOMPARE(person->displayLabel(), QString::fromLatin1("Test"));
    QCOMPARE(person->sectionBucket(), QString::fromLatin1("T"));
    QCOMPARE(spy.count(), 2);

    // change first
    // TODO: eventually where sectionBucket comes from should be a setting
    person->setFirstName("Another");
    QCOMPARE(person->displayLabel(), QString::fromLatin1("Another Test"));
    QCOMPARE(person->sectionBucket(), QString::fromLatin1("A"));
    QCOMPARE(spy.count(), 3);
}

void tst_SeasidePerson::companyName()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);
    QCOMPARE(person->companyName(), QString());
    QSignalSpy spy(person.data(), SIGNAL(companyNameChanged()));
    person->setCompanyName("Test");
    QCOMPARE(spy.count(), 1);
    QCOMPARE(person->companyName(), QString::fromLatin1("Test"));
    QCOMPARE(person->property("companyName").toString(), person->companyName());
}

void tst_SeasidePerson::nickname()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);
    QCOMPARE(person->nickname(), QString());
    QSignalSpy spy(person.data(), SIGNAL(nicknameChanged()));
    person->setNickname("Test");
    QCOMPARE(spy.count(), 1);
    QCOMPARE(person->nickname(), QString::fromLatin1("Test"));
    QCOMPARE(person->property("nickname").toString(), person->nickname());
}

void tst_SeasidePerson::title()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);
    QCOMPARE(person->title(), QString());
    QSignalSpy spy(person.data(), SIGNAL(titleChanged()));
    person->setTitle("Test");
    QCOMPARE(spy.count(), 1);
    QCOMPARE(person->title(), QString::fromLatin1("Test"));
    QCOMPARE(person->property("title").toString(), person->title());
}

void tst_SeasidePerson::favorite()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);
    QVERIFY(!person->favorite());
    QSignalSpy spy(person.data(), SIGNAL(favoriteChanged()));
    person->setFavorite(true);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(person->property("favorite").toBool(), person->favorite());
    QVERIFY(person->favorite());
}

void tst_SeasidePerson::avatarPath()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);
    QCOMPARE(person->avatarPath(), QUrl("image://theme/icon-m-telephony-contact-avatar")); // TODO: should this not really belong in QML level instead of API?
    QSignalSpy spy(person.data(), SIGNAL(avatarPathChanged()));
    person->setAvatarPath(QUrl("http://test.com"));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(person->avatarPath(), QUrl("http://test.com"));
    QCOMPARE(person->property("avatarPath").toUrl(), person->avatarPath());
}

void tst_SeasidePerson::phoneNumbers()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);
    QCOMPARE(person->phoneNumbers(), QStringList());
    QSignalSpy spy(person.data(), SIGNAL(phoneNumbersChanged()));
    person->setPhoneNumbers(QStringList() << "1234" << "5678" << "9101112");
    QCOMPARE(spy.count(), 1);
    QCOMPARE(person->phoneNumbers(), QStringList() << "1234" << "5678" << "9101112");
    QCOMPARE(person->property("phoneNumbers").toStringList(), person->phoneNumbers());
}

void tst_SeasidePerson::phoneTypes()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);

    QCOMPARE(person->phoneNumberTypes(), QList<int>());
    QCOMPARE(person->phoneNumbers(), QStringList());

    person->setPhoneNumbers(QStringList() << "111" << "222"<< "333"<< "444"<< "555");

    QSignalSpy spy(person.data(), SIGNAL(phoneNumberTypesChanged()));

    QList<SeasidePerson::DetailType> phoneTypes;
    phoneTypes.append(SeasidePerson::PhoneHomeType);
    phoneTypes.append(SeasidePerson::PhoneWorkType);
    phoneTypes.append(SeasidePerson::PhoneMobileType);
    phoneTypes.append(SeasidePerson::PhonePagerType);
    phoneTypes.append(SeasidePerson::PhoneFaxType);

    QStringList numbers = person->phoneNumbers();
    QCOMPARE(numbers.count(), 5);
    for (int i=0; i<numbers.count(); i++) {
        person->setPhoneNumberType(i, phoneTypes.at(i));
    }

    // Test that we don't crash here even if we set the type out of bounds.
    QTest::ignoreMessage(QtWarningMsg, "Unable to set type for phone number: invalid index specified. Aborting. ");
    person->setPhoneNumberType(numbers.length(), phoneTypes.at(phoneTypes.length() - 1));

    QCOMPARE(spy.count(), 5);
    QCOMPARE(person->phoneNumbers().count(), 5);
    QCOMPARE(person->phoneNumberTypes().count(), 5);

    QList<int> contactsPhoneTypes = person->phoneNumberTypes();
    for (int i=0; i<contactsPhoneTypes.count(); i++) {
        QVERIFY(contactsPhoneTypes.at(i) == phoneTypes.at(i));
    }
    QCOMPARE(person->property("phoneNumbers").toStringList(), person->phoneNumbers());
}

void tst_SeasidePerson::emailTypes()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);

    QCOMPARE(person->emailAddressTypes(), QList<int>());
    QCOMPARE(person->emailAddresses(), QStringList());

    person->setEmailAddresses(QStringList() << "foo@bar" << "bar@foo"<< "foo@baz");
    QSignalSpy spy(person.data(), SIGNAL(emailAddressTypesChanged()));

    QList<SeasidePerson::DetailType> emailTypes;
    emailTypes.append(SeasidePerson::EmailHomeType);
    emailTypes.append(SeasidePerson::EmailWorkType);
    emailTypes.append(SeasidePerson::EmailOtherType);

    QStringList emails = person->emailAddresses();
    QCOMPARE(emails.count(), 3);
    for (int i=0; i<emails.count(); i++) {
        person->setEmailAddressType(i, emailTypes.at(i));
    }

    // Test that we don't crash here even if we set the type out of bounds.
    QTest::ignoreMessage(QtWarningMsg, "Unable to set type for email address: invalid index specified. Aborting. ");
    person->setEmailAddressType(emails.length(), emailTypes.at(emailTypes.length() - 1));

    QCOMPARE(spy.count(), 3);
    QCOMPARE(person->emailAddresses().count(), 3);
    QCOMPARE(person->emailAddressTypes().count(), 3);

    QList<int> contactsSmailTypes = person->emailAddressTypes();
    for (int i=0; i<contactsSmailTypes.count(); i++) {
        QVERIFY(contactsSmailTypes.at(i) == emailTypes.at(i));
    }
}

void tst_SeasidePerson::emailAddresses()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);
    QCOMPARE(person->emailAddresses(), QStringList());
    QSignalSpy spy(person.data(), SIGNAL(emailAddressesChanged()));
    person->setEmailAddresses(QStringList() << "foo@bar.com" << "moo@cow.com" << "lol@snafu.com");
    QCOMPARE(spy.count(), 1);
    QCOMPARE(person->emailAddresses(), QStringList() << "foo@bar.com" << "moo@cow.com" << "lol@snafu.com");
    QCOMPARE(person->property("emailAddresses").toStringList(), person->emailAddresses());
}

void tst_SeasidePerson::websites()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);
    QCOMPARE(person->websites(), QStringList());
    QSignalSpy spy(person.data(), SIGNAL(websitesChanged()));
    person->setWebsites(QStringList() << "www.example.com" << "www.test.com" << "www.foobar.com");
    QCOMPARE(spy.count(), 1);
    QCOMPARE(person->websites(), QStringList() << "www.example.com" << "www.test.com" << "www.foobar.com");
    QCOMPARE(person->property("websites").toStringList(), person->websites());
}

void tst_SeasidePerson::websiteTypes()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);

    QCOMPARE(person->websiteTypes(), QList<int>());
    QCOMPARE(person->websites(), QStringList());

    person->setWebsites(QStringList() << "www.example.com" << "www.test.com" << "www.foobar.com");
    QSignalSpy spy(person.data(), SIGNAL(websiteTypesChanged()));

    QList<SeasidePerson::DetailType> websiteTypes;
    websiteTypes.append(SeasidePerson::WebsiteHomeType);
    websiteTypes.append(SeasidePerson::WebsiteWorkType);
    websiteTypes.append(SeasidePerson::WebsiteOtherType);

    QStringList websites = person->websites();
    QCOMPARE(websites.count(), 3);
    for (int i = 0; i<websites.count(); i++) {
        person->setWebsiteType(i, websiteTypes.at(i));
    }

    QCOMPARE(spy.count(), 3);
    QCOMPARE(person->websites().count(), 3);
    QCOMPARE(person->websiteTypes().count(), 3);

    QList<int> contactsWebsiteTypes = person->websiteTypes();
    for (int i=0; i<contactsWebsiteTypes.count(); i++) {
        QVERIFY(contactsWebsiteTypes.at(i) == websiteTypes.at(i));
    }
}

void tst_SeasidePerson::birthday()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);
    QCOMPARE(person->birthday(), QDateTime());
    QSignalSpy spy(person.data(), SIGNAL(birthdayChanged()));
    person->setBirthday(QDateTime::fromString("05/01/1980 15:00:00.000", "dd/MM/yyyy hh:mm:ss.zzz"));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(person->birthday(), QDateTime::fromString("05/01/1980 15:00:00.000", "dd/MM/yyyy hh:mm:ss.zzz"));
    QCOMPARE(person->property("birthday").toDateTime(), person->birthday());
    person->resetBirthday();
    QCOMPARE(spy.count(), 2);
    QCOMPARE(person->birthday(), QDateTime());
    QCOMPARE(person->property("birthday").toDateTime(), person->birthday());
}

void tst_SeasidePerson::anniversary()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);
    QCOMPARE(person->anniversary(), QDateTime());
    QSignalSpy spy(person.data(), SIGNAL(anniversaryChanged()));
    person->setAnniversary(QDateTime::fromString("05/01/1980 15:00:00.000", "dd/MM/yyyy hh:mm:ss.zzz"));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(person->anniversary(), QDateTime::fromString("05/01/1980 15:00:00.000", "dd/MM/yyyy hh:mm:ss.zzz"));
    QCOMPARE(person->property("anniversary").toDateTime(), person->anniversary());
    person->resetAnniversary();
    QCOMPARE(spy.count(), 2);
    QCOMPARE(person->anniversary(), QDateTime());
    QCOMPARE(person->property("anniversary").toDateTime(), person->anniversary());
}

void tst_SeasidePerson::address()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);
    QCOMPARE(person->addresses(), QStringList());
    QCOMPARE(person->addressTypes(), QList<int>());
    QSignalSpy spy(person.data(), SIGNAL(addressesChanged()));

    QString address1 = "Street 1\nLocality 1\nRegion 1\nPostcode 1\nCountry 1\nPoBox 1";
    QString address2 = "Street 2\nLocality 2\nRegion 2\nPostcode 2\nCountry 2\nPoBox 2";

    QStringList addresses;
    addresses.append(address1);
    addresses.append(address2);
    person->setAddresses(addresses);
    QCOMPARE(spy.count(), 1);

    addresses = person->addresses();
    QCOMPARE(addresses.count(), 2);
    QCOMPARE(addresses.at(0), QString("Street 1\nLocality 1\nRegion 1\nPostcode 1\nCountry 1\nPoBox 1"));
    QCOMPARE(addresses.at(1), QString("Street 2\nLocality 2\nRegion 2\nPostcode 2\nCountry 2\nPoBox 2"));

    person->setAddressType(0, SeasidePerson::AddressHomeType);
    person->setAddressType(1, SeasidePerson::AddressWorkType);

    QTest::ignoreMessage(QtWarningMsg, "Unable to set type for address: invalid index specified. Aborting. ");
    person->setAddressType(2, SeasidePerson::AddressWorkType);  // Invalid, should not crash.

    QCOMPARE(person->addressTypes().count(), 2);
    QCOMPARE(person->addressTypes().at(0), (int)SeasidePerson::AddressHomeType);
    QCOMPARE(person->addressTypes().at(1), (int)SeasidePerson::AddressWorkType);
}

void tst_SeasidePerson::globalPresenceState()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);
    QCOMPARE(person->globalPresenceState(), SeasidePerson::PresenceUnknown);

    // We can't test any changes, as the change is effected by a DBUS request...
}

void tst_SeasidePerson::complete()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);
    // An empty person is always complete
    QCOMPARE(person->isComplete(), true);

    QSignalSpy spy(person.data(), SIGNAL(completeChanged()));
    person->setComplete(false);
    person->setComplete(true);
    QCOMPARE(spy.count(), 2);
}

void tst_SeasidePerson::marshalling()
{
    QContact contact;

    {
        QContactName nameDetail;
        nameDetail.setFirstName("Hello");
        nameDetail.setLastName("World");
        contact.saveDetail(&nameDetail);
    }

    {
        QContactFavorite favorite;
        favorite.setFavorite(true);
        contact.saveDetail(&favorite);
    }

    QScopedPointer<SeasidePerson> person(new SeasidePerson(contact));
    QCOMPARE(person->firstName(), QString::fromLatin1("Hello"));
    QCOMPARE(person->lastName(), QString::fromLatin1("World"));
    QCOMPARE(person->displayLabel(), QString::fromLatin1("Hello World"));
    QVERIFY(person->favorite());

    QCOMPARE(person->contact(), contact);
}

void tst_SeasidePerson::setContact()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);

    QContact contact;

    {   // ### contactChanged is only emitted if the id differs, not any of the members.
        QContactId contactId;
        contactId.setLocalId(5);
        contact.setId(contactId);
    }

    {
        QContactName nameDetail;
        nameDetail.setFirstName("Hello");
        nameDetail.setLastName("World");
        contact.saveDetail(&nameDetail);
    }

    {
        QSignalSpy spy(person.data(), SIGNAL(firstNameChanged()));
        QSignalSpy spyTwo(person.data(), SIGNAL(lastNameChanged()));
        QSignalSpy spyThree(person.data(), SIGNAL(displayLabelChanged()));
        QSignalSpy spyFour(person.data(), SIGNAL(contactChanged()));
        person->setContact(contact);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spyTwo.count(), 1);
        QCOMPARE(spyThree.count(), 1);
        QCOMPARE(spyFour.count(), 1);

        // change them again, nothing should emit
        person->setContact(contact);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spyTwo.count(), 1);
        QCOMPARE(spyThree.count(), 1);
        QCOMPARE(spyFour.count(), 1);
    }
}

void tst_SeasidePerson::vcard()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);
    person->setFirstName("Star");
    person->setLastName("Fish");
    person->setPhoneNumbers(QStringList() << "12345678");

    QString vcard = person->vCard();

    QVERIFY(vcard.indexOf("N:Fish;Star;") > 0);
    QVERIFY(vcard.indexOf("FN:Star Fish") > 0);
    QVERIFY(vcard.indexOf("TEL:12345678") > 0);
}


// TODO:
// - account URIs/paths (or let contactsd do that?)

#include "tst_seasideperson.moc"
QTEST_APPLESS_MAIN(tst_SeasidePerson)
