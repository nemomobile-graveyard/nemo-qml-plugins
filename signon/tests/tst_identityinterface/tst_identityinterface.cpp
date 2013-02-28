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
    void identifierPending();
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
    void signInOut();
    //methodMechanisms() related invokables are tested in methods().

    // expected usage
    void expectedUsage();
};

void tst_IdentityInterface::identifier()
{
    QScopedPointer<IdentityInterface> identity(new IdentityInterface);
    identity->classBegin();
    identity->componentComplete();
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

    QScopedPointer<IdentityInterface> identityTwo(new IdentityInterface);
    identityTwo->classBegin();
    identityTwo->componentComplete();
    QTRY_COMPARE(identityTwo->status(), IdentityInterface::Initialized);
    QCOMPARE(identityTwo->identifier(), 0);
    QSignalSpy spyTwo(identityTwo.data(), SIGNAL(identifierChanged()));
    identityTwo->setUserName(QString(QLatin1String("test-username-two")));
    identityTwo->setSecret(QString(QLatin1String("test-secret")));
    identityTwo->setCaption(QString(QLatin1String("test-caption")));
    identityTwo->setMethodMechanisms(QString(QLatin1String("password")), QStringList() << QString(QLatin1String("ClientLogin")));
    identityTwo->sync();
    QTRY_COMPARE(spyTwo.count(), 1);
    QVERIFY(identityTwo->identifier() > 0);

    // this one doesn't create a new identity, but references an existing identity
    QScopedPointer<IdentityInterface> identityThree(new IdentityInterface);
    identityThree->classBegin();
    identityThree->setIdentifier(identity->identifier());
    identityThree->componentComplete();
    QTRY_COMPARE(identityThree->status(), IdentityInterface::Initialized);
    QCOMPARE(identityThree->userName(), QLatin1String("test-username"));
    identityThree->setIdentifier(identityTwo->identifier()); // test that you can set it after initialization.
    QCOMPARE(identityThree->status(), IdentityInterface::Initializing);
    QTRY_COMPARE(identityThree->status(), IdentityInterface::Synced);
    QCOMPARE(identityThree->userName(), QLatin1String("test-username-two"));

    // cleanup
    identity->remove();
    identityTwo->remove();
}

void tst_IdentityInterface::identifierPending()
{
    {
        // test pending identifier with zero (creates new account)
        QScopedPointer<IdentityInterface> identity(new IdentityInterface);
        identity->classBegin();
        identity->setIdentifierPending(true);
        QCOMPARE(identity->identifierPending(), true);
        identity->componentComplete(); // should NOT trigger creation of a new identity
        QTest::qWait(1000); // give it enough time to ensure that it's staying initializing.
        QCOMPARE(identity->status(), IdentityInterface::Initializing); // should remain initializing.
        QCOMPARE(identity->identifier(), 0);
        QSignalSpy spy(identity.data(), SIGNAL(identifierChanged()));
        identity->setIdentifier(0); // even though it's the same value, this triggers the creation.
        QTRY_COMPARE(identity->status(), IdentityInterface::Initialized);
        identity->setUserName(QString(QLatin1String("test-username")));
        identity->setSecret(QString(QLatin1String("test-secret")));
        identity->setCaption(QString(QLatin1String("test-caption")));
        identity->setMethodMechanisms(QString(QLatin1String("password")), QStringList() << QString(QLatin1String("ClientLogin")));
        identity->sync();
        QTRY_VERIFY(identity->identifier() > 0);
        QCOMPARE(spy.count(), 1); // should emit when it gets the "real" value.

        // cleanup
        identity->remove();
    }

    {
        // test pending identifier of non-existent identity identifier
        QScopedPointer<IdentityInterface> identity(new IdentityInterface);
        identity->classBegin();
        QCOMPARE(identity->identifierPending(), false);
        identity->setIdentifierPending(true);
        QCOMPARE(identity->identifierPending(), true);
        identity->setIdentifierPending(false);
        QCOMPARE(identity->identifierPending(), false); // should be settable multiple times to avoid binding re-evaluation issues
        identity->setIdentifierPending(true);
        QCOMPARE(identity->identifierPending(), true);
        identity->componentComplete(); // should NOT trigger creation of a new identity, but should stop us from changing identifier pending.
        identity->setIdentifierPending(false); // should fail
        QCOMPARE(identity->identifierPending(), true);
        QTest::qWait(1000); // give it enough time to ensure that it's staying initializing.
        QCOMPARE(identity->status(), IdentityInterface::Initializing); // should remain initializing.
        QCOMPARE(identity->identifier(), 0);
        QSignalSpy spy(identity.data(), SIGNAL(identifierChanged()));
        identity->setIdentifier(99999999);
        QCOMPARE(identity->identifier(), 99999999);
        QTRY_VERIFY(identity->status() != IdentityInterface::Initializing); // should transition out of this state.
        QTRY_VERIFY(identity->error() != IdentityInterface::NoError); // should be an error, since that identifier shouldn't be valid.
        QVERIFY(spy.count() > 0);
    }
}

void tst_IdentityInterface::status()
{
    QScopedPointer<IdentityInterface> identity(new IdentityInterface);
    identity->classBegin();
    QCOMPARE(identity->status(), IdentityInterface::Initializing);
    identity->componentComplete();
    QSignalSpy spy(identity.data(), SIGNAL(statusChanged()));
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(identity->status(), IdentityInterface::Initialized);
    identity->setUserName(QString(QLatin1String("test-username")));
    QCOMPARE(spy.count(), 2);
    QCOMPARE(identity->status(), IdentityInterface::Modified);
    identity->setSecret(QString(QLatin1String("test-secret")));
    identity->sync();
    QTRY_COMPARE(spy.count(), 4); // (Modified)->SyncInProgress->Synced
    QCOMPARE(identity->status(), IdentityInterface::Synced);
    QVERIFY(identity->identifier() > 0); // should have been saved successfully.
    identity->remove();
    QTRY_COMPARE(spy.count(), 6); // (Synced)->SyncInProgress->Invalid
    QCOMPARE(identity->status(), IdentityInterface::Invalid);
}

void tst_IdentityInterface::error()
{
    QScopedPointer<IdentityInterface> identity(new IdentityInterface);
    QCOMPARE(identity->error(), IdentityInterface::NoError);

    // XXX TODO: manually trigger error?
}

void tst_IdentityInterface::errorMessage()
{
    QScopedPointer<IdentityInterface> identity(new IdentityInterface);
    QCOMPARE(identity->errorMessage(), QString());

    // XXX TODO: manually trigger error?
}

void tst_IdentityInterface::userName()
{
    int identityIdentifier = 0;

    // unsaved, ensure that setting the username works
    {
        QScopedPointer<IdentityInterface> identity(new IdentityInterface);
        QSignalSpy spy(identity.data(), SIGNAL(userNameChanged()));
        identity->setUserName(QString(QLatin1String("test-username")));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(identity->userName(), QString(QLatin1String("test-username")));
    }

    // after save, ensure that the username can be read back
    {
        QScopedPointer<IdentityInterface> identity(new IdentityInterface);
        identity->classBegin();
        identity->componentComplete();
        QTRY_COMPARE(identity->status(), IdentityInterface::Initialized);
        QCOMPARE(identity->userName(), QString());
        identity->setUserName(QString(QLatin1String("test-username")));
        QCOMPARE(identity->userName(), QString(QLatin1String("test-username")));
        identity->sync(); // begin sync.
        QCOMPARE(identity->userName(), QString(QLatin1String("test-username")));
        QTRY_COMPARE(identity->status(), IdentityInterface::Synced);
        QCOMPARE(identity->userName(), QString(QLatin1String("test-username")));
        identityIdentifier = identity->identifier();
    }

    // ensure that it can be read from the non-writing instance
    {
        QScopedPointer<IdentityInterface> identity(new IdentityInterface);
        identity->classBegin();
        identity->setIdentifier(identityIdentifier);
        identity->componentComplete();
        QTRY_COMPARE(identity->status(), IdentityInterface::Initialized);
        QCOMPARE(identity->userName(), QString(QLatin1String("test-username")));
        identity->remove(); // cleanup.
    }
}

void tst_IdentityInterface::secret()
{
    QScopedPointer<IdentityInterface> identity(new IdentityInterface);
    QSignalSpy spy(identity.data(), SIGNAL(secretChanged()));
    identity->setSecret(QString(QLatin1String("test-secret")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(identity->secret(), QString(QLatin1String("test-secret")));
}

void tst_IdentityInterface::caption()
{
    QScopedPointer<IdentityInterface> identity(new IdentityInterface);
    QSignalSpy spy(identity.data(), SIGNAL(captionChanged()));
    identity->setCaption(QString(QLatin1String("test-caption")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(identity->caption(), QString(QLatin1String("test-caption")));
}

void tst_IdentityInterface::realms()
{
    QScopedPointer<IdentityInterface> identity(new IdentityInterface);
    QSignalSpy spy(identity.data(), SIGNAL(realmsChanged()));
    QCOMPARE(identity->realms(), QStringList());
    identity->setRealms(QStringList() << QString(QLatin1String("test-realm")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(identity->realms(), QStringList() << QString(QLatin1String("test-realm")));
}

void tst_IdentityInterface::owner()
{
    QScopedPointer<IdentityInterface> identity(new IdentityInterface);
    QSignalSpy spy(identity.data(), SIGNAL(ownerChanged()));
    identity->setOwner(QString(QLatin1String("test-owner")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(identity->owner(), QString(QLatin1String("test-owner")));
}

void tst_IdentityInterface::accessControlList()
{
    QScopedPointer<IdentityInterface> identity(new IdentityInterface);
    QSignalSpy spy(identity.data(), SIGNAL(accessControlListChanged()));
    QCOMPARE(identity->accessControlList(), QStringList());
    identity->setAccessControlList(QStringList() << QString(QLatin1String("test-acl")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(identity->accessControlList(), QStringList() << QString(QLatin1String("test-acl")));
}

void tst_IdentityInterface::methods()
{
    QScopedPointer<IdentityInterface> identity(new IdentityInterface);
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

void tst_IdentityInterface::signInOut()
{
    // test that signIn / signOut work, and that remove() signs out if necessary
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
        QVERIFY(identity->signIn("password", "ClientLogin", QVariantMap())); // start new auth session
        // not expecting a response, as the client login signon plugin may not exist.
        QVERIFY(!identity->signIn("password", "ClientLogin", QVariantMap())); // already in progress
        identity->process(QVariantMap()); // shouldn't crash
        identity->signOut(); // shouldn't crash
        QVERIFY(identity->signIn("password", "ClientLogin", QVariantMap())); // start new auth session
        QSignalSpy spy(identity.data(), SIGNAL(statusChanged()));
        identity->remove(); // remove should also signOut.
        QCOMPARE(spy.count(), 3); // (some signIn() state) -> NotStarted -> SyncInProgress -> Invalid
    }

    // test that dtor signs out if necessary
    int identityToRemove = 0;
    {
        // first, create a new Identity so that we can use it for authentication.
        IdentityInterface *identity = new IdentityInterface;
        identity->classBegin();
        identity->componentComplete();
        QTRY_COMPARE(identity->status(), IdentityInterface::Initialized);
        identity->setUserName(QString(QLatin1String("test-username")));
        identity->setMethodMechanisms(QString(QLatin1String("password")), QStringList() << QString(QLatin1String("ClientLogin")));
        identity->sync(); // begin sync.
        QTRY_COMPARE(identity->status(), IdentityInterface::Synced);
        identityToRemove = identity->identifier();
        QVERIFY(identity->signIn("password", "ClientLogin", QVariantMap())); // start new auth session
        // not expecting a response, as the client login signon plugin may not exist.
        QSignalSpy spy(identity, SIGNAL(statusChanged()));
        delete identity; // delete should also sign out
        QCOMPARE(spy.count(), 1); // (some signIn() state) -> NotStarted
    }

    // clean up
    QScopedPointer<IdentityInterface> identity(new IdentityInterface);
    identity->classBegin();
    identity->setIdentifier(identityToRemove);
    identity->componentComplete();
    QTRY_VERIFY(identity->status() == IdentityInterface::Initialized || identity->status() == IdentityInterface::Synced);
    identity->remove();
}

void tst_IdentityInterface::expectedUsage()
{
    // we expect Identity to be used in QML as a creatable type.
    // eg:
    // Item {
    //     Identity {
    //         id: ident
    //         userName: "username"
    //         secret: "secret"
    //         caption: "caption"
    //         onStatusChanged: {
    //             if (status == Identity.Synced) {
    //                 console.log("successfully created identity with id: " + identifier)
    //             }
    //         }
    //     }
    //
    //     Component.onCompleted: ident.sync() // usually triggered by button press/etc
    // }

    // new identity creation
    QScopedPointer<IdentityInterface> newIdentity(new IdentityInterface);
    newIdentity->classBegin();
    newIdentity->setUserName("test-username");
    newIdentity->setSecret("test-secret");
    newIdentity->setCaption("test-caption");
    newIdentity->componentComplete();
    QTRY_COMPARE(newIdentity->status(), IdentityInterface::Modified);
    newIdentity->sync();
    QTRY_COMPARE(newIdentity->status(), IdentityInterface::Synced);
    //QCOMPARE(newIdentity->userName(), QString(QLatin1String("test-username")));//can't read back after sync.
    //QCOMPARE(newIdentity->secret(), QString(QLatin1String("test-secret")));//can't read back after sync.
    QCOMPARE(newIdentity->caption(), QString(QLatin1String("test-caption")));
    QVERIFY(newIdentity->identifier() > 0); // saved; valid id.

    // existing identity creation
    QScopedPointer<IdentityInterface> existingIdentity(new IdentityInterface);
    existingIdentity->classBegin();
    existingIdentity->setIdentifier(newIdentity->identifier());
    existingIdentity->setCaption("test-caption-modified");
    existingIdentity->componentComplete();
    QTRY_COMPARE(existingIdentity->status(), IdentityInterface::Modified);
    existingIdentity->sync();
    QTRY_COMPARE(existingIdentity->status(), IdentityInterface::Synced);
    QCOMPARE(existingIdentity->caption(), QString(QLatin1String("test-caption-modified")));

    // sadly, newIdentity->caption() won't automatically update.  It needs refresh.
    QCOMPARE(newIdentity->caption(), QString(QLatin1String("test-caption")));
    newIdentity->refresh();
    QTRY_COMPARE(newIdentity->status(), IdentityInterface::Synced);
    QCOMPARE(newIdentity->caption(), QString(QLatin1String("test-caption-modified")));

    // clean up.
    newIdentity->remove();
    existingIdentity->remove(); // may cause issues.  We don't get the signal from the backend :-/
}

#include "tst_identityinterface.moc"
QTEST_MAIN(tst_IdentityInterface)
