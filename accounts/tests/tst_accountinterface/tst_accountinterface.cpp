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
    //enableAccountWithService() in enabledServiceNames()
    //disableAccountWithService() in enabledServiceNames()
};

void tst_AccountInterface::enabled()
{
    Accounts::Manager m;
    Accounts::Account *a = m.createAccount("test-provider");

    QScopedPointer<AccountInterface> account(new AccountInterface(a));
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
    Accounts::Manager m;
    Accounts::Account *a = m.createAccount("test-provider");

    QScopedPointer<AccountInterface> account(new AccountInterface(a));
    QCOMPARE(account->identifier(), 0);
    QSignalSpy spy(account.data(), SIGNAL(identifierChanged()));
    account->setDisplayName("test-display-name");
    account->sync();
    QTRY_COMPARE(spy.count(), 1);
    QVERIFY(account->identifier() > 0);

    // cleanup.
    a->remove();
    a->sync();
}

void tst_AccountInterface::identityIdentifiers()
{
    Accounts::Manager m;
    Accounts::Account *a = m.createAccount("test-provider");

    QVariantMap testData;
    testData.insert(QString(), 0); // no credentials for global account
    testData.insert("test-service", 0); // no credentials for test-service

    QScopedPointer<AccountInterface> account(new AccountInterface(a));
    QCOMPARE(account->identityIdentifiers(), testData);
    QSignalSpy spy(account.data(), SIGNAL(identityIdentifiersChanged()));

    account->setIdentityIdentifier(5, "test-service");
    testData.insert(QString(QLatin1String("test-service")), 5);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(account->identityIdentifier("test-service"), 5);
    QCOMPARE(account->identityIdentifiers(), testData);

    account->setIdentityIdentifier(6);
    testData.insert(QString(), 6);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(account->identityIdentifier(), 6);
    QCOMPARE(account->identityIdentifiers(), testData);

    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced); // wait for sync.
    QCOMPARE(account->identityIdentifier("test-service"), 5); // shouldn't have changed...
    // the "global" credentials id might have changed, if it can't be stored.  Skip testing that.
}

void tst_AccountInterface::providerName()
{
    Accounts::Manager m;
    Accounts::Account *a = m.createAccount("test-provider");

    QScopedPointer<AccountInterface> account(new AccountInterface(a));
    QCOMPARE(account->providerName(), QString(QLatin1String("test-provider")));
}

void tst_AccountInterface::displayName()
{
    Accounts::Manager m;
    Accounts::Account *a = m.createAccount("test-provider");

    QScopedPointer<AccountInterface> account(new AccountInterface(a));
    QCOMPARE(account->displayName(), QString(QLatin1String("")));
    QSignalSpy spy(account.data(), SIGNAL(displayNameChanged()));
    account->setDisplayName(QString(QLatin1String("test-display-name")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(account->displayName(), QString(QLatin1String("test-display-name")));
}

void tst_AccountInterface::supportedServiceNames()
{
    Accounts::Manager m;
    Accounts::Account *a = m.createAccount("test-provider");

    QScopedPointer<AccountInterface> account(new AccountInterface(a));
    QCOMPARE(account->supportedServiceNames(), QStringList() << QString(QLatin1String("test-service")));
}

void tst_AccountInterface::enabledServiceNames()
{
    Accounts::Manager m;
    Accounts::Account *a = m.createAccount("test-provider");

    QScopedPointer<AccountInterface> account(new AccountInterface(a));
    QCOMPARE(account->enabledServiceNames(), QStringList());
    QSignalSpy spy(account.data(), SIGNAL(enabledServiceNamesChanged()));
    account->enableAccountWithService(QString(QLatin1String("test-service")));
    account->sync();
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(account->enabledServiceNames(), QStringList() << QString(QLatin1String("test-service")));
    account->disableAccountWithService(QString(QLatin1String("test-service")));
    account->sync();
    QTRY_COMPARE(spy.count(), 2);
    QCOMPARE(account->enabledServiceNames(), QStringList());

    // cleanup.
    a->remove();
    a->sync();
}

void tst_AccountInterface::configurationValues()
{
    Accounts::Manager m;
    Accounts::Account *a = m.createAccount("test-provider");

    QVariantMap testData;
    QString testKey(QLatin1String("test-key"));
    QVariant testValue(QString(QLatin1String("test-value")));
    testData.insert(testKey, testValue);

    QVariantMap noValueTestData;
    noValueTestData.insert(testKey, QVariant());

    QScopedPointer<AccountInterface> account(new AccountInterface(a));
    QCOMPARE(account->configurationValues(), QVariantMap());
    QSignalSpy spy(account.data(), SIGNAL(configurationValuesChanged()));
    account->setConfigurationValues(testData);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(account->configurationValues(), testData);
    account->setConfigurationValue(testKey, QVariant());
    QCOMPARE(spy.count(), 2);
    QCOMPARE(account->configurationValues(), noValueTestData);
    account->removeConfigurationValue(testKey);
    QCOMPARE(spy.count(), 3);
    QCOMPARE(account->configurationValues(), QVariantMap());
}

void tst_AccountInterface::status()
{
    Accounts::Manager m;
    Accounts::Account *a = m.createAccount("test-provider");

    QScopedPointer<AccountInterface> account(new AccountInterface(a));
    QCOMPARE(account->status(), AccountInterface::Synced);
    QSignalSpy spy(account.data(), SIGNAL(statusChanged()));
    account->setDisplayName(QString(QLatin1String("test-display-name")));
    QCOMPARE(account->status(), AccountInterface::Modified);
    account->sync();
    QTRY_COMPARE(spy.count(), 3); // Synced->Modified->SyncInProgress->Synced.
    QCOMPARE(account->status(), AccountInterface::Synced);

    // cleanup.
    a->remove();
    a->sync();

    QTRY_COMPARE(spy.count(), 4); // Invalid.
    QCOMPARE(account->status(), AccountInterface::Invalid);
}

void tst_AccountInterface::error()
{
    Accounts::Manager m;
    Accounts::Account *a = m.createAccount("test-provider");

    QScopedPointer<AccountInterface> account(new AccountInterface(a));
    QCOMPARE(account->error(), AccountInterface::NoError);

    // XXX TODO: manually trigger an error?
}

void tst_AccountInterface::errorMessage()
{
    Accounts::Manager m;
    Accounts::Account *a = m.createAccount("test-provider");

    QScopedPointer<AccountInterface> account(new AccountInterface(a));
    QCOMPARE(account->errorMessage(), QString());

    // XXX TODO: manually trigger an error?
}


#include "tst_accountinterface.moc"
QTEST_MAIN(tst_AccountInterface)
