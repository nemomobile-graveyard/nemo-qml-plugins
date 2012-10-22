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

#include <QContactFavorite>
#include <QContactName>

#include "seasideperson.h"

QTM_USE_NAMESPACE

class tst_SeasidePerson : public QObject
{
    Q_OBJECT

private slots:
    void firstName();
    void lastName();
    void displayLabel();
    void sectionBucket();
    void companyName();
    void favorite();
    void avatarPath();
    void phoneNumbers();
    void phoneContexts();
    void emailAddresses();
    void marshalling();
    void setContact();
};

void tst_SeasidePerson::firstName()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);
    QCOMPARE(person->firstName(), QString());
    QSignalSpy spy(person.data(), SIGNAL(firstNameChanged()));
    person->setFirstName("Test");
    QCOMPARE(spy.count(), 1);
    QCOMPARE(person->firstName(), QString::fromLatin1("Test"));
}

void tst_SeasidePerson::lastName()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);
    QCOMPARE(person->lastName(), QString());
    QSignalSpy spy(person.data(), SIGNAL(lastNameChanged()));
    person->setLastName("Test");
    QCOMPARE(spy.count(), 1);
    QCOMPARE(person->lastName(), QString::fromLatin1("Test"));
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
    QCOMPARE(person->displayLabel(), QString());
    QCOMPARE(person->sectionBucket(), QString());

    // set first
    person->setLastName("Test");
    QCOMPARE(person->displayLabel(), QString::fromLatin1("Test"));
    QCOMPARE(person->sectionBucket(), QString::fromLatin1("T"));
    QCOMPARE(spy.count(), 1);

    // change first
    // TODO: eventually where sectionBucket comes from should be a setting
    person->setFirstName("Another");
    QCOMPARE(person->displayLabel(), QString::fromLatin1("Another Test"));
    QCOMPARE(person->sectionBucket(), QString::fromLatin1("A"));
    QCOMPARE(spy.count(), 2);
}

void tst_SeasidePerson::companyName()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);
    QCOMPARE(person->companyName(), QString());
    QSignalSpy spy(person.data(), SIGNAL(companyNameChanged()));
    person->setCompanyName("Test");
    QCOMPARE(spy.count(), 1);
    QCOMPARE(person->companyName(), QString::fromLatin1("Test"));
}

void tst_SeasidePerson::favorite()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);
    QVERIFY(!person->favorite());
    QSignalSpy spy(person.data(), SIGNAL(favoriteChanged()));
    person->setFavorite(true);
    QCOMPARE(spy.count(), 1);
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
}

void tst_SeasidePerson::phoneNumbers()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);
    QCOMPARE(person->phoneNumbers(), QStringList());
    QSignalSpy spy(person.data(), SIGNAL(phoneNumbersChanged()));
    person->setPhoneNumbers(QStringList() << "1234" << "5678" << "9101112");
    QCOMPARE(spy.count(), 1);
    QCOMPARE(person->phoneNumbers(), QStringList() << "1234" << "5678" << "9101112");
}

void tst_SeasidePerson::phoneContexts() {
    QScopedPointer<SeasidePerson> person(new SeasidePerson);

    QCOMPARE(person->phoneContexts(), QStringList());
    QCOMPARE(person->phoneNumbers(), QStringList());

    person->setPhoneNumbers(QStringList() << "111" << "222"<< "333"<< "444"<< "555");

    QSignalSpy spy(person.data(), SIGNAL(phoneContextsChanged()));
    person->setPhoneContexts(QStringList() << SeasidePerson::SP_CONTEXT_PHONE_HOME
                                           << SeasidePerson::SP_CONTEXT_PHONE_WORK
                                           << SeasidePerson::SP_CONTEXT_PHONE_MOBILE
                                           << SeasidePerson::SP_CONTEXT_PHONE_FAX
                                           << SeasidePerson::SP_CONTEXT_PHONE_PAGER);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(person->phoneContexts(), QStringList() << SeasidePerson::SP_CONTEXT_PHONE_HOME
                                                    << SeasidePerson::SP_CONTEXT_PHONE_WORK
                                                    << SeasidePerson::SP_CONTEXT_PHONE_MOBILE
                                                    << SeasidePerson::SP_CONTEXT_PHONE_FAX
                                                    << SeasidePerson::SP_CONTEXT_PHONE_PAGER);
    QCOMPARE(person->phoneNumbers().count(), 5);
}

void tst_SeasidePerson::emailAddresses()
{
    QScopedPointer<SeasidePerson> person(new SeasidePerson);
    QCOMPARE(person->emailAddresses(), QStringList());
    QSignalSpy spy(person.data(), SIGNAL(emailAddressesChanged()));
    person->setEmailAddresses(QStringList() << "foo@bar.com" << "moo@cow.com" << "lol@snafu.com");
    QCOMPARE(spy.count(), 1);
    QCOMPARE(person->emailAddresses(), QStringList() << "foo@bar.com" << "moo@cow.com" << "lol@snafu.com");
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
        person->setContact(contact);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spyTwo.count(), 1);
        QCOMPARE(spyThree.count(), 1);

        // change them again, nothing should emit
        person->setContact(contact);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spyTwo.count(), 1);
        QCOMPARE(spyThree.count(), 1);
    }
}


// TODO:
// - account URIs/paths (or let contactsd do that?)

#include "tst_seasideperson.moc"
QTEST_APPLESS_MAIN(tst_SeasidePerson)
