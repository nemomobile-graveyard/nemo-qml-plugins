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

#include "serviceaccountinterface.h"

#include "accountmanagerinterface.h"
#include "accountinterface.h"
#include "authdatainterface.h"
#include "serviceinterface.h"
#include "providerinterface.h"

//libaccounts-qt
#include <Accounts/Manager>
#include <Accounts/Account>
#include <Accounts/AccountService>

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

class tst_ServiceAccountInterface : public QObject
{
    Q_OBJECT

private slots:
    //properties
    void enabled();
    void identifier();
    void authData();  // also tests the AuthDataInterface type.
    void service();
    void provider();
    void configurationValues(); // also tests the configuration value invokables.

    // invokables
    void encodeDecode();

    // expected usage.
    void expectedUsage();
};

void tst_ServiceAccountInterface::enabled()
{
    QScopedPointer<AccountInterface> account(new AccountInterface);
    account->classBegin();
    account->setProviderName("test-provider");
    account->setDisplayName("test-display-name");
    account->sync(); // pending sync - empty account (ie, no display name) won't get saved.
    QCOMPARE(account->identifier(), 0);
    account->componentComplete(); // will construct new account.
    QTRY_VERIFY(account->identifier() > 0);

    account->enableWithService("test-service2");
    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);

    {
        Accounts::Manager m;
        Accounts::Account *a = m.account(account->identifier());
        QVERIFY(a);
        Accounts::AccountService as(a, m.service("test-service2"));
        QScopedPointer<ServiceAccountInterface> sai(new ServiceAccountInterface(&as, ServiceAccountInterface::DoesNotHaveOwnership));
        QCOMPARE(sai->enabled(), true);
    }

    account->disableWithService("test-service2");
    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);

    {
        Accounts::Manager m;
        Accounts::Account *a = m.account(account->identifier());
        QVERIFY(a);
        Accounts::AccountService as(a, m.service("test-service2"));
        QScopedPointer<ServiceAccountInterface> sai(new ServiceAccountInterface(&as, ServiceAccountInterface::DoesNotHaveOwnership));
        QCOMPARE(sai->enabled(), false);
    }

    // cleanup.
    account->remove();
}

void tst_ServiceAccountInterface::identifier()
{
    QScopedPointer<AccountInterface> account(new AccountInterface);
    account->classBegin();
    account->setProviderName("test-provider");
    account->setDisplayName("test-display-name");
    account->sync(); // pending sync - empty account (ie, no display name) won't get saved.
    QCOMPARE(account->identifier(), 0);
    account->componentComplete(); // will construct new account.
    QTRY_VERIFY(account->identifier() > 0);
    account->enableWithService("test-service2");
    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);

    {
        Accounts::Manager m;
        Accounts::Account *a = m.account(account->identifier());
        QVERIFY(a);
        Accounts::AccountService as(a, m.service("test-service2"));
        QScopedPointer<ServiceAccountInterface> sai(new ServiceAccountInterface(&as, ServiceAccountInterface::DoesNotHaveOwnership));
        QCOMPARE(sai->identifier(), account->identifier());
    }

    // cleanup.
    account->remove();
}

void tst_ServiceAccountInterface::authData()
{
    QScopedPointer<AccountInterface> account(new AccountInterface);
    account->classBegin();
    account->setProviderName("test-provider");
    account->setDisplayName("test-display-name");
    account->sync(); // pending sync - empty account (ie, no display name) won't get saved.
    QCOMPARE(account->identifier(), 0);
    account->componentComplete(); // will construct new account.
    QTRY_VERIFY(account->identifier() > 0);
    account->enableWithService("test-service2");
    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);

    {
        Accounts::Manager m;
        Accounts::Account *a = m.account(account->identifier());
        QVERIFY(a);
        Accounts::AccountService as(a, m.service("test-service2"));
        QScopedPointer<ServiceAccountInterface> sai(new ServiceAccountInterface(&as, ServiceAccountInterface::DoesNotHaveOwnership));
        AuthDataInterface *adi = sai->authData();
        QVERIFY(adi != 0);
        QCOMPARE(adi->method(), QString(QLatin1String("password")));
        QCOMPARE(adi->mechanism(), QString(QLatin1String("password")));
        QCOMPARE(adi->identityIdentifier(), 0); // cannot be tested independently of signon.
        QCOMPARE(adi->parameters().value("testing"), QVariant()); // cannot be tested independently of signon.
    }

    // cleanup.
    account->remove();
}

void tst_ServiceAccountInterface::service()
{
    QScopedPointer<AccountInterface> account(new AccountInterface);
    account->classBegin();
    account->setProviderName("test-provider");
    account->setDisplayName("test-display-name");
    account->sync(); // pending sync - empty account (ie, no display name) won't get saved.
    QCOMPARE(account->identifier(), 0);
    account->componentComplete(); // will construct new account.
    QTRY_VERIFY(account->identifier() > 0);
    account->enableWithService("test-service2");
    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);

    {
        Accounts::Manager m;
        Accounts::Account *a = m.account(account->identifier());
        QVERIFY(a);
        Accounts::AccountService as(a, m.service("test-service2"));
        QScopedPointer<ServiceAccountInterface> sai(new ServiceAccountInterface(&as, ServiceAccountInterface::DoesNotHaveOwnership));
        QVERIFY(sai->service());
        QCOMPARE(sai->service()->name(), QString(QLatin1String("test-service2")));
    }

    // cleanup.
    account->remove();
}

void tst_ServiceAccountInterface::provider()
{
    QScopedPointer<AccountInterface> account(new AccountInterface);
    account->classBegin();
    account->setProviderName("test-provider");
    account->setDisplayName("test-display-name");
    account->sync(); // pending sync - empty account (ie, no display name) won't get saved.
    QCOMPARE(account->identifier(), 0);
    account->componentComplete(); // will construct new account.
    QTRY_VERIFY(account->identifier() > 0);
    account->enableWithService("test-service2");
    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);

    {
        Accounts::Manager m;
        Accounts::Account *a = m.account(account->identifier());
        QVERIFY(a);
        Accounts::AccountService as(a, m.service("test-service2"));
        QScopedPointer<ServiceAccountInterface> sai(new ServiceAccountInterface(&as, ServiceAccountInterface::DoesNotHaveOwnership));
        QVERIFY(sai->provider());
        QCOMPARE(sai->provider()->name(), QString(QLatin1String("test-provider")));
    }

    // cleanup.
    account->remove();
}

void tst_ServiceAccountInterface::configurationValues()
{
    QScopedPointer<AccountInterface> account(new AccountInterface);
    account->classBegin();
    account->setProviderName("test-provider");
    account->setDisplayName("test-display-name");
    account->sync(); // pending sync - empty account (ie, no display name) won't get saved.
    QCOMPARE(account->identifier(), 0);
    account->componentComplete(); // will construct new account.
    QTRY_VERIFY(account->identifier() > 0);
    account->enableWithService("test-service2");
    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    account->setConfigurationValue(QString(QLatin1String("unrelated")), QVariant(QString(QLatin1String("testvalue"))));
    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    QCOMPARE(account->configurationValues().value("unrelated"), QVariant(QString(QLatin1String("testvalue"))));

    {
        Accounts::Manager m;
        Accounts::Account *a = m.account(account->identifier());
        QVERIFY(a);
        Accounts::AccountService as(a, m.service("test-service2"));
        QScopedPointer<ServiceAccountInterface> sai(new ServiceAccountInterface(&as, ServiceAccountInterface::DoesNotHaveOwnership));
        QSignalSpy spy(sai.data(), SIGNAL(configurationValuesChanged()));
        account->setConfigurationValue(QLatin1String("testing"), QLatin1String("first"), QLatin1String("test-service2"));
        account->sync();
        QTest::qWait(2000); // after 2 seconds, it should have "stabilised".
        int emitCount = spy.count();
        QTRY_VERIFY(emitCount >= 1); // it can emit any number of times, so we can't use QTRY_COMPARE here.
        sai->setConfigurationValue(QString(QLatin1String("testing")), QVariant(QString("test"))); // will auto sync.
        QTRY_COMPARE(spy.count(), emitCount + 1);
        QCOMPARE(sai->configurationValues().value("testing"), QVariant(QString("test")));
        QCOMPARE(sai->unrelatedValues().value("unrelated"), QVariant(QString("testvalue")));
    }

    // cleanup.
    account->remove();
}

void tst_ServiceAccountInterface::encodeDecode()
{
    QScopedPointer<AccountInterface> account(new AccountInterface);
    account->classBegin();
    account->setProviderName("test-provider");
    account->setDisplayName("test-display-name");
    account->sync(); // pending sync - empty account (ie, no display name) won't get saved.
    QCOMPARE(account->identifier(), 0);
    account->componentComplete(); // will construct new account.
    QTRY_VERIFY(account->identifier() > 0);

    {
        Accounts::Manager m;
        Accounts::Account *a = m.account(account->identifier());
        QVERIFY(a);
        Accounts::AccountService as(a, m.service("test-service2"));

        QScopedPointer<ServiceAccountInterface> sai(new ServiceAccountInterface(&as, ServiceAccountInterface::DoesNotHaveOwnership));

        // XXX TODO: test UTF8 inputs
        QString inputOne = QLatin1String("AAAaaaAbbBzzzZZ");
        QString inputTwo = QLatin1String("n3m0abcDEF1234567890");
        QString inputThree = QLatin1String("-123-abcd-000-ZYXA-");

        QString b641 = sai->encodeConfigurationValue(inputOne);                    // default scheme is b64
        QString b6412 = sai->encodeConfigurationValue(inputOne, "base64");
        QString b6413 = sai->encodeConfigurationValue(inputOne, "base64", "test"); // key should be ignored
        QString b642 = sai->encodeConfigurationValue(inputTwo);
        QString b643 = sai->encodeConfigurationValue(inputThree);

        QString rot1 = sai->encodeConfigurationValue(inputOne, "rot");
        QString rot12 = sai->encodeConfigurationValue(inputOne, "rot", "test"); // key should be ignored
        QString rot2 = sai->encodeConfigurationValue(inputTwo, "rot");
        QString rot3 = sai->encodeConfigurationValue(inputThree, "rot");

        QString xor1 = sai->encodeConfigurationValue(inputOne, "xor");             // default key is "nemo"
        QString xor12 = sai->encodeConfigurationValue(inputOne, "xor", "nemo");
        QString xor13 = sai->encodeConfigurationValue(inputOne, "xor", "test");
        QString xor2 = sai->encodeConfigurationValue(inputTwo, "xor");
        QString xor3 = sai->encodeConfigurationValue(inputThree, "xor");

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

        QCOMPARE(sai->decodeConfigurationValue(b641, "base64", "test"), inputOne);
        QCOMPARE(sai->decodeConfigurationValue(b641, "base64"), inputOne);
        QCOMPARE(sai->decodeConfigurationValue(b641), inputOne);
        QCOMPARE(sai->decodeConfigurationValue(b642), inputTwo);

        QCOMPARE(sai->decodeConfigurationValue(rot1, "rot", "test"), inputOne);
        QCOMPARE(sai->decodeConfigurationValue(rot1, "rot"), inputOne);
        QCOMPARE(sai->decodeConfigurationValue(rot2, "rot"), inputTwo);
        QCOMPARE(sai->decodeConfigurationValue(rot3, "rot"), inputThree);

        QCOMPARE(sai->decodeConfigurationValue(xor13, "xor", "test"), inputOne);
        QCOMPARE(sai->decodeConfigurationValue(xor12, "xor", "nemo"), inputOne);
        QCOMPARE(sai->decodeConfigurationValue(xor1, "xor"), inputOne);
        QCOMPARE(sai->decodeConfigurationValue(xor2, "xor"), inputTwo);
        QCOMPARE(sai->decodeConfigurationValue(xor3, "xor"), inputThree);
    }

    // cleanup.
    account->remove();
}

void tst_ServiceAccountInterface::expectedUsage()
{
    // I'm still not certain.
}

#include "tst_serviceaccountinterface.moc"
QTEST_MAIN(tst_ServiceAccountInterface)
