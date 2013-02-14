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
    void serviceConfigurationValues();
    //these are already tested:
    //sync() in identifier()
    //enableWithService() in enabledServiceNames()
    //disableWithService() in enabledServiceNames()

    void loadSavedAccount();
    void encodeDecode();

    // expected usage
    void expectedUsage();
};

void tst_AccountInterface::enabled()
{
    // first, without account creation / sync
    QScopedPointer<AccountInterface> account(new AccountInterface);
    account->classBegin();
    account->setProviderName("test-provider");
    account->setDisplayName("test-display-name");
    QCOMPARE(account->enabled(), false);
    QSignalSpy spy(account.data(), SIGNAL(enabledChanged()));
    account->setEnabled(true);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(account->enabled(), true);
    account->setEnabled(false);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(account->enabled(), false);

    // now, with sync.
    account->setEnabled(true);
    account->sync(); // pending sync
    account->componentComplete(); // will construct new account.
    QTRY_COMPARE(account->status(), AccountInterface::Synced); // wait for sync.
    QVERIFY(account->enabled());

    // and ensure that it's globally enabled in the database.
    Accounts::Manager m;
    Accounts::Account *a = m.account(account->identifier());
    QVERIFY(a->enabled());

    // disable it
    account->setEnabled(false);
    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced); // wait for sync.

    // ensure that it's globally disabled in the database.
    QVERIFY(!a->enabled());

    // cleanup.
    account->remove();
}

void tst_AccountInterface::identifier()
{
    QScopedPointer<AccountInterface> account(new AccountInterface);

    // create a new account
    {
        QSignalSpy spy(account.data(), SIGNAL(identifierChanged()));
        account->classBegin();
        account->setProviderName("test-provider");
        account->setDisplayName("test-display-name");
        account->sync(); // pending sync - empty account (ie, no display name) won't get saved.
        QCOMPARE(account->identifier(), 0);
        account->componentComplete(); // will construct new account.
        QTRY_COMPARE(spy.count(), 1);
        QVERIFY(account->identifier() > 0);

        // and ensure that it exists in the database
        Accounts::Manager m;
        Accounts::Account *a = m.account(account->identifier());
        QCOMPARE(a->displayName(), QLatin1String("test-display-name"));
    }

    // existing account identifier
    {
        QScopedPointer<AccountInterface> existing(new AccountInterface);
        existing->classBegin();
        existing->setIdentifier(account->identifier());
        existing->componentComplete();
        QTRY_COMPARE(existing->status(), AccountInterface::Initialized);
        QCOMPARE(existing->displayName(), QLatin1String("test-display-name"));
    }

    // existing account identifier set after initialization
    {
        QScopedPointer<AccountInterface> existing(new AccountInterface);
        existing->classBegin();
        existing->componentComplete();
        QTRY_COMPARE(existing->status(), AccountInterface::Invalid);
        existing->setIdentifier(account->identifier());
        QCOMPARE(existing->status(), AccountInterface::Initializing);
        QTRY_COMPARE(existing->status(), AccountInterface::Initialized);
        QCOMPARE(existing->displayName(), QLatin1String("test-display-name"));
    }

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
    QTRY_COMPARE(account->status(), AccountInterface::Synced);

    QCOMPARE(account->enabledServiceNames(), QStringList());
    account->enableWithService(QString(QLatin1String("test-service2")));
    QCOMPARE(account->status(), AccountInterface::Modified);
    int currCount = spy.count();
    account->sync();
    // note: the change signal can be emitted an arbitrary number of times,
    // depending on how many signals the backend emits (one by one).
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    QTRY_VERIFY(spy.count() > currCount); // can get multiple signals, depending on supported service names etc.
    QTRY_COMPARE(account->enabledServiceNames(), QStringList() << QString(QLatin1String("test-service2")));

    // ensure that the account really has been enabled as we expect.
    Accounts::Manager m;
    Accounts::Account *a = m.account(account->identifier());
    QVERIFY(a->enabledServices().contains(m.service(QLatin1String("test-service2"))));

    // now disable and test again
    account->disableWithService(QString(QLatin1String("test-service2")));
    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    QCOMPARE(account->enabledServiceNames(), QStringList());

    // ensure that the account really has been disabled as we expect.
    QVERIFY(!a->enabledServices().contains(m.service(QLatin1String("test-service2"))));

    // cleanup
    account->remove();
}

void tst_AccountInterface::configurationValues()
{
    QVariantMap testData;
    QString testKey(QLatin1String("test-key"));
    QVariant testStrListValue(QStringList() << QLatin1String("first") << QLatin1String("second"));
    QVariant testStrValue(QString(QLatin1String("test-value")));
    QVariant testBoolValue(true);
    QVariant testIntValue(-5);
    QVariant testQuintValue(quint64(0xaaaaaaaaaaaa));
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
    account->setConfigurationValue(testKey, testStrListValue);
    QCOMPARE(spy.count(), 7);
    QCOMPARE(account->configurationValues().value(testKey), testStrListValue);

    // ensure that configuration values can be saved.
    account->sync(); // pending sync.
    account->componentComplete(); // will create new account.
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    QCOMPARE(account->configurationValues().value(testKey), testStrListValue);
    account->setConfigurationValue(testKey, testStrValue);
    account->sync();
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
    // and ensure that stringlist configuration values are reported correctly.
    QString testGroup = QLatin1String("test-group");
    Accounts::Manager m;
    Accounts::Account *a = m.account(account->identifier());
    QVERIFY(a != 0);
    a->beginGroup(testGroup);
    a->setValue(testKey, testStrValue);
    a->endGroup();
    a->setValue(testKey, testStrListValue);
    a->sync();

    // account doesn't emit signals on configuration values changed...
    // we really need a "refresh" function, similar to the one in Identity.
    QScopedPointer<AccountInterface> existingAccount(new AccountInterface);
    existingAccount->classBegin();
    existingAccount->setIdentifier(account->identifier());
    existingAccount->componentComplete(); // will load existing account
    QTRY_COMPARE(existingAccount->status(), AccountInterface::Initialized);
    QCOMPARE(existingAccount->configurationValues().value(QString("%1/%2").arg(testGroup).arg(testKey)), testStrValue);
    QCOMPARE(existingAccount->configurationValues().value(testKey), testStrListValue);

    // and ensure that changes are really synced
    account->setConfigurationValue(testKey, testStrValue);
    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    QVariant expectString(QVariant::String);
    a->value(testKey, expectString); // expectString is an in-out argument.
    QCOMPARE(expectString, testStrValue);
    account->setConfigurationValue(testKey, testStrListValue);
    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    QVariant expectStringList(QVariant::StringList);
    a->value(testKey, expectStringList); // expectStringList is an in-out argument.
    QCOMPARE(expectStringList, testStrListValue);

    // cleanup.
    account->remove();
}

void tst_AccountInterface::serviceConfigurationValues()
{
    QVariantMap testData;
    QString testKey(QLatin1String("service-test-key")); // different to key in previous test, to avoid overlap.
    QVariant testStrListValue(QStringList() << QLatin1String("first") << QLatin1String("second"));
    QVariant testStrValue(QString(QLatin1String("test-value")));
    QVariant testBoolValue(true);
    QVariant testIntValue(-5);
    QVariant testQuintValue(quint64(0xaaaaaaaaaaaa));
    testData.insert(testKey, testStrValue);
    QString testServiceName = QLatin1String("test-service2");

    QVariantMap noValueTestData;
    noValueTestData.insert(testKey, QVariant());

    QScopedPointer<AccountInterface> account(new AccountInterface);
    account->classBegin();
    account->setProviderName("test-provider");
    QCOMPARE(account->configurationValues(testServiceName), QVariantMap());
    QSignalSpy spy(account.data(), SIGNAL(configurationValuesChanged()));
    account->setConfigurationValues(testData, testServiceName);
    int currentCount = spy.count();
    QCOMPARE(account->configurationValues(testServiceName), testData);
    account->removeConfigurationValue(testKey, testServiceName);
    QCOMPARE(spy.count(), currentCount); // shouldn't emit - the configurationValuesChanged() signal is for global config values only.
    QCOMPARE(account->configurationValues(testServiceName), QVariantMap());

    // invalid values
    account->setConfigurationValue(testKey, QVariant(QColor(Qt::black)), testServiceName);
    QCOMPARE(spy.count(), currentCount);
    QCOMPARE(account->configurationValues(testServiceName), QVariantMap()); // not set.
    account->setConfigurationValue(testKey, QVariant(), testServiceName);
    QCOMPARE(spy.count(), currentCount);
    QCOMPARE(account->configurationValues(testServiceName), QVariantMap()); // not set.

    // bool, int, quint64 and string should all work.
    account->setConfigurationValue(testKey, testBoolValue, testServiceName);
    QCOMPARE(spy.count(), currentCount);
    QCOMPARE(account->configurationValues(testServiceName).value(testKey), testBoolValue);
    account->setConfigurationValue(testKey, testIntValue, testServiceName);
    QCOMPARE(spy.count(), currentCount);
    QCOMPARE(account->configurationValues(testServiceName).value(testKey), testIntValue);
    account->setConfigurationValue(testKey, testQuintValue, testServiceName);
    QCOMPARE(spy.count(), currentCount);
    QCOMPARE(account->configurationValues(testServiceName).value(testKey), testQuintValue);
    account->setConfigurationValue(testKey, testStrValue, testServiceName);
    QCOMPARE(spy.count(), currentCount);
    QCOMPARE(account->configurationValues(testServiceName).value(testKey), testStrValue);
    account->setConfigurationValue(testKey, testStrListValue, testServiceName);
    QCOMPARE(spy.count(), currentCount);
    QCOMPARE(account->configurationValues(testServiceName).value(testKey), testStrListValue);

    // ensure that configuration values can be saved.
    account->sync(); // pending sync.
    account->componentComplete(); // will create new account.
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    QCOMPARE(account->configurationValues(testServiceName).value(testKey), testStrListValue);
    account->setConfigurationValue(testKey, testStrValue, testServiceName);
    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    QCOMPARE(account->configurationValues(testServiceName).value(testKey), testStrValue);
    account->setConfigurationValue(testKey, testQuintValue, testServiceName);
    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    QCOMPARE(account->configurationValues(testServiceName).value(testKey), testQuintValue);
    account->setConfigurationValue(testKey, testIntValue, testServiceName);
    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    QCOMPARE(account->configurationValues(testServiceName).value(testKey), testIntValue);
    account->setConfigurationValue(testKey, testBoolValue, testServiceName);
    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    QCOMPARE(account->configurationValues(testServiceName).value(testKey), testBoolValue);

    // ensure that configuration values from subgroups are reported correctly.
    // and ensure that stringlist configuration values are reported correctly.
    QString testGroup = QLatin1String("test-group");
    Accounts::Manager m;
    Accounts::Account *a = m.account(account->identifier());
    QVERIFY(a != 0);
    Accounts::Service s = m.service(testServiceName);
    QVERIFY(s.isValid());
    a->selectService(s);
    a->beginGroup(testGroup);
    a->setValue(testKey, testStrValue);
    a->endGroup();
    a->setValue(testKey, testStrListValue);
    a->sync();

    // account doesn't emit signals on configuration values changed...
    // we really need a "refresh" function, similar to the one in Identity.
    QScopedPointer<AccountInterface> existingAccount(new AccountInterface);
    existingAccount->classBegin();
    existingAccount->setIdentifier(account->identifier());
    existingAccount->componentComplete(); // will load existing account
    QTRY_COMPARE(existingAccount->status(), AccountInterface::Initialized);
    QCOMPARE(existingAccount->configurationValues(testServiceName).value(QString("%1/%2").arg(testGroup).arg(testKey)), testStrValue);
    QCOMPARE(existingAccount->configurationValues(testServiceName).value(testKey), testStrListValue);

    // and ensure that changes are really synced
    account->setConfigurationValue(testKey, testStrValue, testServiceName);
    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    QVariant expectString(QVariant::String);
    a->value(testKey, expectString); // expectString is an in-out parameter.
    QCOMPARE(expectString, testStrValue);
    account->setConfigurationValue(testKey, testStrListValue, testServiceName);
    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    QVariant expectStringList(QVariant::StringList);
    a->value(testKey, expectStringList); // expectStringList is an in-out parameter.
    QCOMPARE(expectStringList, testStrListValue);

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

void tst_AccountInterface::loadSavedAccount()
{
    QScopedPointer<AccountInterface> account(new AccountInterface);
    account->classBegin();
    account->setProviderName("test-provider");
    account->setDisplayName("test-display-name");
    account->sync();
    account->componentComplete();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    QTRY_VERIFY(account->supportedServiceNames().contains(QString(QLatin1String("test-service2"))));
    account->enableWithService(QString(QLatin1String("test-service2")));
    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);

    QScopedPointer<AccountInterface> readOnlyAccount(new AccountInterface);
    QSignalSpy spyIdentityIdentifiers(readOnlyAccount.data(), SIGNAL(identityIdentifiersChanged()));
    QSignalSpy spyProviderName(readOnlyAccount.data(), SIGNAL(providerNameChanged()));
    QSignalSpy spyDisplayName(readOnlyAccount.data(), SIGNAL(displayNameChanged()));
    QSignalSpy spySupportedServiceNames(readOnlyAccount.data(), SIGNAL(supportedServiceNamesChanged()));
    QSignalSpy spyEnabledServiceNames(readOnlyAccount.data(), SIGNAL(enabledServiceNamesChanged()));

    readOnlyAccount->classBegin();
    readOnlyAccount->setIdentifier(account->identifier());
    readOnlyAccount->componentComplete();
    QTRY_COMPARE(readOnlyAccount->status(), AccountInterface::Initialized);
    QCOMPARE(readOnlyAccount->providerName(), account->providerName());
    QCOMPARE(readOnlyAccount->displayName(), account->displayName());
    QCOMPARE(readOnlyAccount->identifier(), account->identifier());
    QCOMPARE(readOnlyAccount->supportedServiceNames(), account->supportedServiceNames());
    QCOMPARE(readOnlyAccount->enabledServiceNames(), account->enabledServiceNames());

    QCOMPARE(spyIdentityIdentifiers.count(), 1);
    QCOMPARE(spyProviderName.count(), 1);
    QCOMPARE(spyDisplayName.count(), 1);
    QCOMPARE(spySupportedServiceNames.count(), 1);
    QCOMPARE(spyEnabledServiceNames.count(), 1);

    account->remove();
}

void tst_AccountInterface::encodeDecode()
{
    QScopedPointer<AccountInterface> a(new AccountInterface);

    // XXX TODO: test UTF8 inputs
    QString inputOne = QLatin1String("AAAaaaAbbBzzzZZ");
    QString inputTwo = QLatin1String("n3m0abcDEF1234567890");
    QString inputThree = QLatin1String("-123-abcd-000-ZYXA-");

    QString b641 = a->encodeConfigurationValue(inputOne);                    // default scheme is b64
    QString b6412 = a->encodeConfigurationValue(inputOne, "base64");
    QString b6413 = a->encodeConfigurationValue(inputOne, "base64", "test"); // key should be ignored
    QString b642 = a->encodeConfigurationValue(inputTwo);
    QString b643 = a->encodeConfigurationValue(inputThree);

    QString rot1 = a->encodeConfigurationValue(inputOne, "rot");
    QString rot12 = a->encodeConfigurationValue(inputOne, "rot", "test"); // key should be ignored
    QString rot2 = a->encodeConfigurationValue(inputTwo, "rot");
    QString rot3 = a->encodeConfigurationValue(inputThree, "rot");

    QString xor1 = a->encodeConfigurationValue(inputOne, "xor");             // default key is "nemo"
    QString xor12 = a->encodeConfigurationValue(inputOne, "xor", "nemo");
    QString xor13 = a->encodeConfigurationValue(inputOne, "xor", "test");
    QString xor2 = a->encodeConfigurationValue(inputTwo, "xor");
    QString xor3 = a->encodeConfigurationValue(inputThree, "xor");

    QCOMPARE(b641, b6412);
    QCOMPARE(b641, b6413);
    QVERIFY(b641 != b642);
    QVERIFY(b641 != b643);
    QVERIFY(b642 != b643);
    QCOMPARE(b641, QLatin1String("QUFBYWFhQWJiQnp6elpa"));
    QCOMPARE(b642, QLatin1String("bjNtMGFiY0RFRjEyMzQ1Njc4OTA="));
    QCOMPARE(b643, QLatin1String("LTEyMy1hYmNkLTAwMC1aWVhBLQ=="));

    QCOMPARE(rot1, rot12);
    QVERIFY(rot1 != rot2);
    QVERIFY(rot1 != rot3);
    QVERIFY(rot2 != rot3);
    QCOMPARE(rot1, QLatin1String("UVZIRV1cTG9ZYFR0XXt+RXV9gnQ="));
    QCOMPARE(rot2, QLatin1String("YmtQd1FMTHBhOVxRXndTiF2LY0Rif3lLZ21bWA=="));
    QCOMPARE(rot3, QLatin1String("TFVHfFF+N29hdlh2WGFPhl1UQ3Rra35ZZGpXWA=="));

    QCOMPARE(xor1, xor12);
    QVERIFY(xor1 != xor13);
    QVERIFY(xor1 != xor2);
    QVERIFY(xor1 != xor3);
    QVERIFY(xor2 != xor3);
    QCOMPARE(xor1, QLatin1String("PzArLTcyKwc/MicGPwsdWQsJHQ4="));
    QCOMPARE(xor13, QLatin1String("JTA1Ni0yNRwlMjkdJQsDQhEJAxU="));
    QCOMPARE(xor2, QLatin1String("DA8jGyMiKwY3VT8pPA8oFiMfPF4gDw5bITEsUg=="));
    QCOMPARE(xor3, QLatin1String("IjEoFiMcXAc3CCMEIjEsGCMmXA45MwUtIjRQUg=="));

    QCOMPARE(a->decodeConfigurationValue(b641, "base64", "test"), inputOne);
    QCOMPARE(a->decodeConfigurationValue(b641, "base64"), inputOne);
    QCOMPARE(a->decodeConfigurationValue(b641), inputOne);
    QCOMPARE(a->decodeConfigurationValue(b642), inputTwo);

    QCOMPARE(a->decodeConfigurationValue(rot1, "rot", "test"), inputOne);
    QCOMPARE(a->decodeConfigurationValue(rot1, "rot"), inputOne);
    QCOMPARE(a->decodeConfigurationValue(rot2, "rot"), inputTwo);
    QCOMPARE(a->decodeConfigurationValue(rot3, "rot"), inputThree);

    QCOMPARE(a->decodeConfigurationValue(xor13, "xor", "test"), inputOne);
    QCOMPARE(a->decodeConfigurationValue(xor12, "xor", "nemo"), inputOne);
    QCOMPARE(a->decodeConfigurationValue(xor1, "xor"), inputOne);
    QCOMPARE(a->decodeConfigurationValue(xor2, "xor"), inputTwo);
    QCOMPARE(a->decodeConfigurationValue(xor3, "xor"), inputThree);
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
