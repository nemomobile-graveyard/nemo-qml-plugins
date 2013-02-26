/*
 * Copyright (C) 2012 Jolla Ltd. <chris.adams@jollamobile.com>
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

#include "serviceaccountidentityinterface.h"
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

class tst_ServiceAccountIdentityInterface : public QObject
{
    Q_OBJECT

private slots:
    // properties
    void identifier();
    void status();
    void statusMessage();
    void error();
    void errorMessage();
    void methods(); // also tests methodMechanisms()

    // invokables
    void signIn();
    void process();
    void signOut();

    // expected usage
    void expectedUsage();
};

void tst_ServiceAccountIdentityInterface::identifier()
{
    // first, create a new Identity so that we can use it for authentication.
    QScopedPointer<IdentityInterface> identity(new IdentityInterface);
    identity->classBegin();
    identity->componentComplete();
    QTRY_COMPARE(identity->status(), IdentityInterface::Initialized);
    identity->setUserName(QString(QLatin1String("test-username")));
    identity->sync(); // begin sync.
    QTRY_COMPARE(identity->status(), IdentityInterface::Synced);
    int identityIdentifier = identity->identifier();
    QVERIFY(identityIdentifier > 0); // ensure save succeeded.

    // second, create another new Identity so that we can try post-initialization identifier change
    QScopedPointer<IdentityInterface> identityTwo(new IdentityInterface);
    identityTwo->classBegin();
    identityTwo->componentComplete();
    QTRY_COMPARE(identityTwo->status(), IdentityInterface::Initialized);
    identityTwo->setUserName(QString(QLatin1String("test-username-two")));
    identityTwo->sync(); // begin sync.
    QTRY_COMPARE(identityTwo->status(), IdentityInterface::Synced);
    int identityIdentifierTwo = identityTwo->identifier();
    QVERIFY(identityIdentifierTwo > 0); // ensure save succeeded.

    QScopedPointer<ServiceAccountIdentityInterface> saidentity(new ServiceAccountIdentityInterface);
    QCOMPARE(saidentity->identifier(), 0);
    QSignalSpy spy(saidentity.data(), SIGNAL(identifierChanged()));
    saidentity->setIdentifier(identityIdentifier);
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(saidentity->identifier(), identityIdentifier);
    saidentity->setIdentifier(identityIdentifier + 999999); // some invalid identifier
    QCOMPARE(saidentity->status(), ServiceAccountIdentityInterface::Initializing);
    QTRY_COMPARE(saidentity->status(), ServiceAccountIdentityInterface::Error); // invalid identifier
    saidentity->setIdentifier(identityIdentifierTwo);
    QCOMPARE(saidentity->status(), ServiceAccountIdentityInterface::Initializing);
    QTRY_COMPARE(saidentity->status(), ServiceAccountIdentityInterface::Initialized);

    // cleanup
    identity->remove();
    identityTwo->remove();
}

void tst_ServiceAccountIdentityInterface::status()
{
    // first, create a new Identity so that we can use it for authentication.
    QScopedPointer<IdentityInterface> identity(new IdentityInterface);
    identity->classBegin();
    identity->componentComplete();
    QTRY_COMPARE(identity->status(), IdentityInterface::Initialized);
    identity->setUserName(QString(QLatin1String("test-username")));
    identity->sync(); // begin sync.
    QTRY_COMPARE(identity->status(), IdentityInterface::Synced);
    QCOMPARE(identity->userName(), QString(QLatin1String("test-username")));
    int identityIdentifier = identity->identifier();

    QScopedPointer<ServiceAccountIdentityInterface> saidentity(new ServiceAccountIdentityInterface);
    QCOMPARE(saidentity->identifier(), 0);
    QCOMPARE(saidentity->status(), ServiceAccountIdentityInterface::Invalid); // until identifier is set.
    QSignalSpy spy(saidentity.data(), SIGNAL(statusChanged()));
    saidentity->setIdentifier(identityIdentifier);
    QCOMPARE(saidentity->status(), ServiceAccountIdentityInterface::Initializing);
    QCOMPARE(spy.count(), 1);
    QTRY_COMPARE(saidentity->status(), ServiceAccountIdentityInterface::Initialized);
    QTRY_COMPARE(spy.count(), 2);

    // cleanup
    identity->remove();
}

void tst_ServiceAccountIdentityInterface::statusMessage()
{
    QScopedPointer<ServiceAccountIdentityInterface> saidentity(new ServiceAccountIdentityInterface);
    QCOMPARE(saidentity->statusMessage(), QString());
    // status messages are only available during the sign-in process.
}

void tst_ServiceAccountIdentityInterface::error()
{
    QScopedPointer<ServiceAccountIdentityInterface> saidentity(new ServiceAccountIdentityInterface);
    QCOMPARE(saidentity->error(), ServiceAccountIdentityInterface::NoError);

    // XXX TODO: manually trigger error?
}

void tst_ServiceAccountIdentityInterface::errorMessage()
{
    QScopedPointer<ServiceAccountIdentityInterface> saidentity(new ServiceAccountIdentityInterface);
    QCOMPARE(saidentity->errorMessage(), QString());

    // XXX TODO: manually trigger error?
}

void tst_ServiceAccountIdentityInterface::methods()
{
    // first, create a new Identity so that we can use it for authentication.
    QScopedPointer<IdentityInterface> identity(new IdentityInterface);
    identity->classBegin();
    identity->componentComplete();
    QTRY_COMPARE(identity->status(), IdentityInterface::Initialized);
    identity->setUserName(QString(QLatin1String("test-username")));
    identity->setMethodMechanisms(QString(QLatin1String("password")), QStringList() << QString(QLatin1String("ClientLogin")));
    identity->sync(); // begin sync.
    QTRY_COMPARE(identity->status(), IdentityInterface::Synced);
    int identityIdentifier = identity->identifier();

    QScopedPointer<ServiceAccountIdentityInterface> saidentity(new ServiceAccountIdentityInterface);
    saidentity->setIdentifier(identityIdentifier);
    QTRY_COMPARE(saidentity->status(), ServiceAccountIdentityInterface::Initialized);
    QCOMPARE(saidentity->methods(), QStringList() << QString(QLatin1String("password")));
    QCOMPARE(saidentity->methodMechanisms("password"), QStringList() << QString(QLatin1String("ClientLogin")));

    // cleanup
    identity->remove();    
}

void tst_ServiceAccountIdentityInterface::signIn()
{
    // first, create a new Identity so that we can use it for authentication.
    QScopedPointer<IdentityInterface> identity(new IdentityInterface);
    identity->classBegin();
    identity->componentComplete();
    QTRY_COMPARE(identity->status(), IdentityInterface::Initialized);
    identity->setUserName(QString(QLatin1String("test-username")));
    identity->setMethodMechanisms(QString(QLatin1String("password")), QStringList() << QString(QLatin1String("ClientLogin")));
    identity->sync(); // begin sync.
    QTRY_COMPARE(identity->status(), IdentityInterface::Synced);
    int identityIdentifier = identity->identifier();

    QScopedPointer<ServiceAccountIdentityInterface> saidentity(new ServiceAccountIdentityInterface);
    saidentity->setIdentifier(identityIdentifier);
    QTRY_COMPARE(saidentity->status(), ServiceAccountIdentityInterface::Initialized);
    QCOMPARE(saidentity->methods(), QStringList() << QString(QLatin1String("password")));
    QCOMPARE(saidentity->methodMechanisms("password"), QStringList() << QString(QLatin1String("ClientLogin")));
    QVERIFY(saidentity->signIn("password", "ClientLogin", QVariantMap()));
    // not expecting a response, as the client login signon plugin may not exist.

    // cleanup
    identity->remove();
}

void tst_ServiceAccountIdentityInterface::process()
{
    // Can't test this in isolation without appropriate signon plugins (oauth2)
    QSKIP("requires oauth2 signon plugins", SkipSingle);
}

void tst_ServiceAccountIdentityInterface::signOut()
{
    // Can't test this in isolation without appropriate signon plugins (oauth2)
    QSKIP("requires oauth2 signon plugins", SkipSingle);
}

void tst_ServiceAccountIdentityInterface::expectedUsage()
{
    // we expect Identity to be used in QML as a creatable type.
    // eg:
    // Item {
    //      AccountManager { id: acm }
    //      ServiceAccountIdentity {
    //         id: sai
    //         identifier: existingIdentityId
    //         onStatusChanged: {
    //             if (status == Identity.Initialized) {
    //                 var serviceAccount = acm.serviceAccount(existingAccountId)
    //                 signIn("oauth2", "HMAC-SHA1", serviceAccount.authData.parameters)
    //             }
    //         }
    //         onResponseReceived: {
    //             // retrieve tokens from "data" signal parameter, process() if required.
    //         }
    //     }
    // }
    QSKIP("requires oauth2 signon plugins", SkipSingle);
}

#include "tst_serviceaccountidentityinterface.moc"
QTEST_MAIN(tst_ServiceAccountIdentityInterface)
