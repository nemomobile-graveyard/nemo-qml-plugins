/*
 * Copyright (C) 2012 Jolla Mobile <chris.adams@jollamobile.com>
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

#include "identityinterface.h"

//libsignon-qt
#include <SignOn/Identity>

// Will try to wait for the condition while allowing event processing
#define QTRY_VERIFY(__expr) \
    do { \
        const int __step = 50; \
        const int __timeout = 5000; \
        if (!(__expr)) { \
            QTest::qWait(0); \
        } \
        for (int __i = 0; __i < __timeout && !(__expr); __i+=__step) { \
            QTest::qWait(__step); \
        } \
        QVERIFY(__expr); \
    } while (0)

// Will try to wait for the condition while allowing event processing
#define QTRY_COMPARE(__expr, __expected) \
    do { \
        const int __step = 50; \
        const int __timeout = 5000; \
        if ((__expr) != (__expected)) { \
            QTest::qWait(0); \
        } \
        for (int __i = 0; __i < __timeout && ((__expr) != (__expected)); __i+=__step) { \
            QTest::qWait(__step); \
        } \
        QCOMPARE(__expr, __expected); \
    } while (0)

class tst_IdentityInterface : public QObject
{
    Q_OBJECT

private slots:
    //properties
    void identifier();
    void status();
    void error();
    void errorMessage();
    void userName();
    void secret();
    void caption();
    void realms();
    void owner();
    void accessControlList();
    void methods();

    // invokables
    //methodMechanisms() related invokables are tested in methods().
};

void tst_IdentityInterface::identifier()
{
    QScopedPointer<SignOn::Identity> i(SignOn::Identity::newIdentity());

    QScopedPointer<IdentityInterface> identity(new IdentityInterface(i.data()));
    QTRY_COMPARE(identity->status(), IdentityInterface::Initialized);
    QCOMPARE(identity->identifier(), 0);
    QSignalSpy spy(identity.data(), SIGNAL(identifierChanged()));
    identity->setUserName(QString(QLatin1String("test-username")));
    identity->setSecret(QString(QLatin1String("test-secret")));
    identity->setCaption(QString(QLatin1String("test-caption")));
    identity->setMethodMechanisms(QString(QLatin1String("password")), QStringList() << QString(QLatin1String("ClientLogin")));
    identity->sync();
    QTRY_COMPARE(spy.count(), 1);
    QVERIFY(identity->identifier() > 0);

    // cleanup
    i->remove();
}

void tst_IdentityInterface::status()
{
    QScopedPointer<SignOn::Identity> i(SignOn::Identity::newIdentity());

    QScopedPointer<IdentityInterface> identity(new IdentityInterface(i.data()));
    QCOMPARE(identity->status(), IdentityInterface::RefreshInProgress);
    QSignalSpy spy(identity.data(), SIGNAL(statusChanged()));
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(identity->status(), IdentityInterface::Initialized);
    identity->setUserName(QString(QLatin1String("test-username")));
    QCOMPARE(spy.count(), 2);
    QCOMPARE(identity->status(), IdentityInterface::Modified);
    identity->setSecret(QString(QLatin1String("test-secret")));
    identity->sync();
    QTRY_COMPARE(spy.count(), 4); // Modified->SyncInProgress->Synced
    QCOMPARE(identity->status(), IdentityInterface::Synced);

    i->remove();
    QTRY_COMPARE(spy.count(), 5);
    QCOMPARE(identity->status(), IdentityInterface::Invalid);
}

void tst_IdentityInterface::error()
{
    QScopedPointer<SignOn::Identity> i(SignOn::Identity::newIdentity());

    QScopedPointer<IdentityInterface> identity(new IdentityInterface(i.data()));
    QTRY_COMPARE(identity->status(), IdentityInterface::Initialized);
    QCOMPARE(identity->error(), IdentityInterface::NoError);

    // XXX TODO: manually trigger error?
}

void tst_IdentityInterface::errorMessage()
{
    QScopedPointer<SignOn::Identity> i(SignOn::Identity::newIdentity());

    QScopedPointer<IdentityInterface> identity(new IdentityInterface(i.data()));
    QTRY_COMPARE(identity->status(), IdentityInterface::Initialized);
    QCOMPARE(identity->errorMessage(), QString());

    // XXX TODO: manually trigger error?
}

void tst_IdentityInterface::userName()
{
    QScopedPointer<SignOn::Identity> i(SignOn::Identity::newIdentity());

    QScopedPointer<IdentityInterface> identity(new IdentityInterface(i.data()));
    QTRY_COMPARE(identity->status(), IdentityInterface::Initialized);
    QSignalSpy spy(identity.data(), SIGNAL(userNameChanged()));
    identity->setUserName(QString(QLatin1String("test-username")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(identity->userName(), QString(QLatin1String("test-username")));
}

void tst_IdentityInterface::secret()
{
    QScopedPointer<SignOn::Identity> i(SignOn::Identity::newIdentity());

    QScopedPointer<IdentityInterface> identity(new IdentityInterface(i.data()));
    QTRY_COMPARE(identity->status(), IdentityInterface::Initialized);
    QSignalSpy spy(identity.data(), SIGNAL(secretChanged()));
    identity->setSecret(QString(QLatin1String("test-secret")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(identity->secret(), QString(QLatin1String("test-secret")));
}

void tst_IdentityInterface::caption()
{
    QScopedPointer<SignOn::Identity> i(SignOn::Identity::newIdentity());

    QScopedPointer<IdentityInterface> identity(new IdentityInterface(i.data()));
    QTRY_COMPARE(identity->status(), IdentityInterface::Initialized);
    QSignalSpy spy(identity.data(), SIGNAL(captionChanged()));
    identity->setCaption(QString(QLatin1String("test-caption")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(identity->caption(), QString(QLatin1String("test-caption")));
}

void tst_IdentityInterface::realms()
{
    QScopedPointer<SignOn::Identity> i(SignOn::Identity::newIdentity());

    QScopedPointer<IdentityInterface> identity(new IdentityInterface(i.data()));
    QTRY_COMPARE(identity->status(), IdentityInterface::Initialized);
    QSignalSpy spy(identity.data(), SIGNAL(realmsChanged()));
    QCOMPARE(identity->realms(), QStringList());
    identity->setRealms(QStringList() << QString(QLatin1String("test-realm")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(identity->realms(), QStringList() << QString(QLatin1String("test-realm")));
}

void tst_IdentityInterface::owner()
{
    QScopedPointer<SignOn::Identity> i(SignOn::Identity::newIdentity());

    QScopedPointer<IdentityInterface> identity(new IdentityInterface(i.data()));
    QTRY_COMPARE(identity->status(), IdentityInterface::Initialized);
    QSignalSpy spy(identity.data(), SIGNAL(ownerChanged()));
    identity->setOwner(QString(QLatin1String("test-owner")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(identity->owner(), QString(QLatin1String("test-owner")));
}

void tst_IdentityInterface::accessControlList()
{
    QScopedPointer<SignOn::Identity> i(SignOn::Identity::newIdentity());

    QScopedPointer<IdentityInterface> identity(new IdentityInterface(i.data()));
    QTRY_COMPARE(identity->status(), IdentityInterface::Initialized);
    QSignalSpy spy(identity.data(), SIGNAL(accessControlListChanged()));
    QCOMPARE(identity->accessControlList(), QStringList());
    identity->setAccessControlList(QStringList() << QString(QLatin1String("test-acl")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(identity->accessControlList(), QStringList() << QString(QLatin1String("test-acl")));
}

void tst_IdentityInterface::methods()
{
    QScopedPointer<SignOn::Identity> i(SignOn::Identity::newIdentity());

    QScopedPointer<IdentityInterface> identity(new IdentityInterface(i.data()));
    QTRY_COMPARE(identity->status(), IdentityInterface::Initialized);
    QSignalSpy spy(identity.data(), SIGNAL(methodsChanged()));
    QCOMPARE(identity->methods(), QStringList());
    identity->setMethodMechanisms(QString(QLatin1String("password")), QStringList() << QString(QLatin1String("ClientLogin")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(identity->methods(), QStringList() << QString(QLatin1String("password")));
    QCOMPARE(identity->methodMechanisms(QString(QLatin1String("password"))), QStringList() << QString(QLatin1String("ClientLogin")));
    identity->removeMethod(QString(QLatin1String("password")));
    QCOMPARE(spy.count(), 2);
    QCOMPARE(identity->methods(), QStringList());
}

#include "tst_identityinterface.moc"
QTEST_MAIN(tst_IdentityInterface)
