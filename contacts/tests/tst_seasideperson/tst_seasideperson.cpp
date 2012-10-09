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

#include "seasideperson.h"

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

// TODO:
// - phone numbers
// - email addresses
// - account URIs/paths (or let contactsd do that?)
// - test QContact creation (and creation from a QContact)

#include "tst_seasideperson.moc"
QTEST_APPLESS_MAIN(tst_SeasidePerson)
