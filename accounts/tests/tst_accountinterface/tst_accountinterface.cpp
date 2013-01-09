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

#include "accountinterface.h"

//libaccounts-qt
#include <Accounts/Manager>
#include <Accounts/Account>

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

class tst_AccountInterface : public QObject
{
    Q_OBJECT

private slots:
    //properties
    void enabled();
    void identifier();
    void identityIdentifiers();
    void providerName();
    void displayName();
    void supportedServiceNames();
    void enabledServiceNames();
    void configurationValues();
    void status();
    void error();
    void errorMessage();
    //invokables
    //these are already tested:
    //sync() in identifier()
    //enableWithService() in enabledServiceNames()
    //disableWithService() in enabledServiceNames()

    // expected usage
    void expectedUsage();
};

void tst_AccountInterface::enabled()
{
    QScopedPointer<AccountInterface> account(new AccountInterface);
    QCOMPARE(account->enabled(), false);
    QSignalSpy spy(account.data(), SIGNAL(enabledChanged()));
    account->setEnabled(true);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(account->enabled(), true);
    account->setEnabled(false);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(account->enabled(), false);
}

void tst_AccountInterface::identifier()
{
    QScopedPointer<AccountInterface> account(new AccountInterface);
    QSignalSpy spy(account.data(), SIGNAL(identifierChanged()));
    account->classBegin();
    account->setProviderName("test-provider");
    account->setDisplayName("test-display-name");
    account->sync(); // pending sync - empty account (ie, no display name) won't get saved.
    QCOMPARE(account->identifier(), 0);
    account->componentComplete(); // will construct new account.
    QTRY_COMPARE(spy.count(), 1);
    QVERIFY(account->identifier() > 0);

    // cleanup.
    account->remove();
}

void tst_AccountInterface::identityIdentifiers()
{
    QScopedPointer<AccountInterface> account(new AccountInterface);
    QSignalSpy spy(account.data(), SIGNAL(identityIdentifiersChanged()));
    account->classBegin();
    account->setProviderName("test-provider");
    account->componentComplete(); // will construct new account.

    QCOMPARE(account->identityIdentifier(), 0);
    QTRY_COMPARE(account->identityIdentifiers().value("test-service2").toInt(), 0);
    int sigCount = spy.count();

    account->setIdentityIdentifier(5, "test-service2");
    sigCount++;
    QCOMPARE(spy.count(), sigCount);
    QCOMPARE(account->identityIdentifier("test-service2"), 5);
    QCOMPARE(account->identityIdentifiers().value("test-service2").toInt(), 5);

    account->setIdentityIdentifier(6);
    sigCount++;
    QCOMPARE(spy.count(), sigCount);
    QCOMPARE(account->identityIdentifier(), 6);

    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced); // wait for sync.
    QCOMPARE(account->identityIdentifier("test-service2"), 5); // shouldn't have changed...
    // the "global" credentials id might have changed, if it can't be stored.  Skip testing that.

    account->remove();
}

void tst_AccountInterface::providerName()
{
    QScopedPointer<AccountInterface> account(new AccountInterface);
    account->setProviderName("test-provider");
    QCOMPARE(account->providerName(), QString(QLatin1String("test-provider")));
    QCOMPARE(account->error(), AccountInterface::NoError);
    // can't set both identifier and providerName:
    account->setIdentifier(5);
    QCOMPARE(account->error(), AccountInterface::ConflictingProviderError);
}

void tst_AccountInterface::displayName()
{
    QScopedPointer<AccountInterface> account(new AccountInterface);
    account->classBegin();
    account->setProviderName("test-provider");
    QCOMPARE(account->displayName(), QString(QLatin1String("")));
    QSignalSpy spy(account.data(), SIGNAL(displayNameChanged()));
    account->setDisplayName(QString(QLatin1String("test-display-name")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(account->displayName(), QString(QLatin1String("test-display-name")));
    account->sync(); // pending sync.
    account->componentComplete(); // will construct new account.
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    account->setDisplayName(QString(QLatin1String("test-display-name-two")));
    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    QCOMPARE(account->displayName(), QString(QLatin1String("test-display-name-two")));
    account->remove();
}

void tst_AccountInterface::supportedServiceNames()
{
    QScopedPointer<AccountInterface> account(new AccountInterface);
    account->classBegin();
    account->setProviderName("test-provider");
    account->componentComplete(); // will construct new account.
    QTRY_VERIFY(account->supportedServiceNames().contains(QString(QLatin1String("test-service2"))));
    account->remove();
}

void tst_AccountInterface::enabledServiceNames()
{
    QScopedPointer<AccountInterface> account(new AccountInterface);
    QSignalSpy spy(account.data(), SIGNAL(enabledServiceNamesChanged()));
    account->classBegin();
    account->setProviderName("test-provider");
    account->setDisplayName("test-display-name");
    account->sync(); // pending sync - empty account (ie, no display name) won't get saved.
    account->componentComplete(); // will construct new account.
    QCOMPARE(account->enabledServiceNames(), QStringList());
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    account->enableWithService(QString(QLatin1String("test-service2")));
    QCOMPARE(account->status(), AccountInterface::Modified);
    account->sync();
    // note: the change signal can be emitted an arbitrary number of times,
    // depending on how many signals the backend emits (one by one).
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    QCOMPARE(account->enabledServiceNames(), QStringList() << QString(QLatin1String("test-service2")));
    account->disableWithService(QString(QLatin1String("test-service2")));
    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    QCOMPARE(account->enabledServiceNames(), QStringList());
    account->remove();
}

void tst_AccountInterface::configurationValues()
{
    QVariantMap testData;
    QString testKey(QLatin1String("test-key"));
    QVariant testStrValue(QString(QLatin1String("test-value")));
    QVariant testBoolValue(true);
    QVariant testIntValue(-5);
    QVariant testQuintValue(0xaaaaaaaaaaaa);
    testData.insert(testKey, testStrValue);

    QVariantMap noValueTestData;
    noValueTestData.insert(testKey, QVariant());

    QScopedPointer<AccountInterface> account(new AccountInterface);
    account->classBegin();
    account->setProviderName("test-provider");
    QCOMPARE(account->configurationValues(), QVariantMap());
    QSignalSpy spy(account.data(), SIGNAL(configurationValuesChanged()));
    account->setConfigurationValues(testData);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(account->configurationValues(), testData);
    account->removeConfigurationValue(testKey);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(account->configurationValues(), QVariantMap());

    // invalid values
    account->setConfigurationValue(testKey, QVariant(QColor(Qt::black)));
    QCOMPARE(spy.count(), 2); // no change signal
    QCOMPARE(account->configurationValues(), QVariantMap()); // not set.
    account->setConfigurationValue(testKey, QVariant());
    QCOMPARE(spy.count(), 2); // no change signal
    QCOMPARE(account->configurationValues(), QVariantMap()); // not set.

    // bool, int, quint64 and string should all work.
    account->setConfigurationValue(testKey, testBoolValue);
    QCOMPARE(spy.count(), 3);
    QCOMPARE(account->configurationValues().value(testKey), testBoolValue);
    account->setConfigurationValue(testKey, testIntValue);
    QCOMPARE(spy.count(), 4);
    QCOMPARE(account->configurationValues().value(testKey), testIntValue);
    account->setConfigurationValue(testKey, testQuintValue);
    QCOMPARE(spy.count(), 5);
    QCOMPARE(account->configurationValues().value(testKey), testQuintValue);
    account->setConfigurationValue(testKey, testStrValue);
    QCOMPARE(spy.count(), 6);
    QCOMPARE(account->configurationValues().value(testKey), testStrValue);

    // ensure that configuration values can be saved.
    account->sync(); // pending sync.
    account->componentComplete(); // will create new account.
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    QCOMPARE(account->configurationValues().value(testKey), testStrValue);
    account->setConfigurationValue(testKey, testQuintValue);
    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    QCOMPARE(account->configurationValues().value(testKey), testQuintValue);
    account->setConfigurationValue(testKey, testIntValue);
    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    QCOMPARE(account->configurationValues().value(testKey), testIntValue);
    account->setConfigurationValue(testKey, testBoolValue);
    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    QCOMPARE(account->configurationValues().value(testKey), testBoolValue);

    // ensure that configuration values from subgroups are reported correctly.
    QString testGroup = QLatin1String("test-group");
    Accounts::Manager m;
    Accounts::Account *a = m.account(account->identifier());
    QVERIFY(a != 0);
    a->beginGroup(testGroup);
    a->setValue(testKey, testStrValue);
    a->endGroup();
    a->sync();

    // account doesn't emit signals on configuration values changed...
    // we really need a "refresh" function, similar to the one in Identity.
    QScopedPointer<AccountInterface> existingAccount(new AccountInterface);
    existingAccount->classBegin();
    existingAccount->setIdentifier(account->identifier());
    existingAccount->componentComplete(); // will load existing account
    QTRY_COMPARE(existingAccount->status(), AccountInterface::Initialized);
    QCOMPARE(existingAccount->configurationValues().value(QString("%1/%2").arg(testGroup).arg(testKey)), testStrValue);

    // cleanup.
    account->remove();
}

void tst_AccountInterface::status()
{
    QScopedPointer<AccountInterface> account(new AccountInterface);
    QSignalSpy spy(account.data(), SIGNAL(statusChanged()));
    account->classBegin();
    QCOMPARE(account->status(), AccountInterface::Initializing);
    account->setProviderName("test-provider");
    account->setDisplayName(QString(QLatin1String("test-display-name")));
    QCOMPARE(account->status(), AccountInterface::Initializing); // despite modifications, should still be initializing, until componentComplete().
    account->sync(); // trigger pending sync().
    account->componentComplete(); // will construct new account.
    // now we return to event loop.
    // Status should transition: (Initializing) -> Initialized -> Modified
    //                         -> SyncInProgress -> Synced.
    QTRY_COMPARE(spy.count(), 4);
    QCOMPARE(account->status(), AccountInterface::Synced);
    account->setDisplayName(QString(QLatin1String("test-display-name-two")));
    QCOMPARE(spy.count(), 5);
    QCOMPARE(account->status(), AccountInterface::Modified);
    account->sync();
    QTRY_COMPARE(spy.count(), 7); // SyncInProgress->Synced.
    QCOMPARE(account->status(), AccountInterface::Synced);
    QVERIFY(account->identifier() > 0); // should have saved the account successfully.

    // cleanup.
    Accounts::Manager m;
    Accounts::Account *a = m.account(account->identifier());
    QVERIFY(a != 0);
    a->remove();
    a->sync();

    QTRY_COMPARE(spy.count(), 8); // Invalid.
    QCOMPARE(account->status(), AccountInterface::Invalid);
}

void tst_AccountInterface::error()
{
    QScopedPointer<AccountInterface> account(new AccountInterface);
    account->setIdentifier(10);
    QCOMPARE(account->error(), AccountInterface::NoError);
    account->setProviderName("test-provider");
    QCOMPARE(account->error(), AccountInterface::ConflictingProviderError);
}

void tst_AccountInterface::errorMessage()
{
    QScopedPointer<AccountInterface> account(new AccountInterface);
    QCOMPARE(account->errorMessage(), QString());
    // XXX TODO: error message for conflicting provider error?
}


void tst_AccountInterface::expectedUsage()
{
    // it's meant to be used in QML as a creatable type.
    // generally, they'll do:
    // Item {
    //     Account {
    //         id: account
    //         displayName: "account name"
    //         providerName: "provider"
    //         onStatusChanged: {
    //             if (status == Account.Synced) {
    //                 console.log("created account with id: " + identifier)
    //             }
    //         }
    //     }
    //
    //     Component.onCompleted: account.sync() // usually triggered by button press/etc
    // }

    // first, as a "new" account.  No identifier, valid providerName.
    QScopedPointer<AccountInterface> newAccount(new AccountInterface); // we reuse it in the existing test
    newAccount->classBegin();
    newAccount->setProviderName("test-provider");
    newAccount->setDisplayName("test-account");
    newAccount->enableWithService("test-service2");
    QCOMPARE(newAccount->status(), AccountInterface::Initializing);
    newAccount->componentComplete();
    QTRY_COMPARE(newAccount->status(), AccountInterface::Modified); // outstanding modifications.
    newAccount->sync(); // common case is to trigger sync after completes.
    QTRY_COMPARE(newAccount->status(), AccountInterface::Synced);
    QCOMPARE(newAccount->providerName(), QString(QLatin1String("test-provider")));
    QCOMPARE(newAccount->displayName(), QString(QLatin1String("test-account")));
    QCOMPARE(newAccount->enabled(), true); // enabled by default.
    QCOMPARE(newAccount->enabledServiceNames(), QStringList() << QString(QLatin1String("test-service2")));
    QVERIFY(newAccount->identifier() > 0); // saved; valid id.

    // second, as an "existing" account.  Valid identifier.
    QScopedPointer<AccountInterface> existingAccount(new AccountInterface);
    existingAccount->classBegin();
    existingAccount->setIdentifier(newAccount->identifier());
    existingAccount->setDisplayName("test-account-modified");
    QCOMPARE(existingAccount->status(), AccountInterface::Initializing);
    existingAccount->componentComplete();
    QTRY_COMPARE(existingAccount->status(), AccountInterface::Modified);
    existingAccount->sync(); // common case is to trigger sync before completes.
    QTRY_COMPARE(existingAccount->status(), AccountInterface::Synced);
    QCOMPARE(existingAccount->identifier(), newAccount->identifier()); // same id
    QCOMPARE(existingAccount->providerName(), QString(QLatin1String("test-provider")));
    QCOMPARE(existingAccount->displayName(), QString(QLatin1String("test-account-modified")));

    // now, it should signal the change to this account.
    QTRY_COMPARE(newAccount->displayName(), QString(QLatin1String("test-account-modified")));

    // clean them both up.
    newAccount->remove(); // removing one should remove both.
    QTRY_COMPARE(newAccount->status(), AccountInterface::Invalid);
    QTRY_COMPARE(existingAccount->status(), AccountInterface::Invalid);
}


#include "tst_accountinterface.moc"
QTEST_MAIN(tst_AccountInterface)
