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

#include "account-model.h"
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

class tst_AccountModel : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void addAccount();
    void removeAccount();
    void updateAccount();
};

Q_DECLARE_METATYPE(QModelIndex)

void tst_AccountModel::initTestCase()
{
    qRegisterMetaType<QModelIndex>("QModelIndex");
}

void tst_AccountModel::addAccount()
{
    AccountModel model;
    int prevCount = model.rowCount();

    QScopedPointer<AccountInterface> account(new AccountInterface);
    account->classBegin();
    account->setProviderName("test-provider");
    account->setDisplayName("some display name");
    account->sync();
    account->componentComplete();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    QCOMPARE(model.rowCount(), prevCount+1);

    Accounts::Manager m;
    Accounts::Provider provider = m.provider(account->providerName());

    QCOMPARE(model.data(model.index(0), AccountModel::AccountIdRole).toInt(), account->identifier());
    QCOMPARE(model.data(model.index(0), AccountModel::AccountDisplayNameRole).toString(), account->displayName());
    QCOMPARE(model.data(model.index(0), AccountModel::AccountIconRole).toString(), provider.iconName());
    QCOMPARE(model.data(model.index(0), AccountModel::ProviderNameRole).toString(), account->providerName());
    QCOMPARE(model.data(model.index(0), AccountModel::ProviderDisplayNameRole).toString(), provider.displayName());

    account->remove();
}

void tst_AccountModel::removeAccount()
{
    AccountModel model;
    int prevCount = model.rowCount();

    QScopedPointer<AccountInterface> account(new AccountInterface);
    account->classBegin();
    account->setProviderName("test-provider");
    account->sync();
    account->componentComplete();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    QCOMPARE(model.rowCount(), prevCount+1);

    account->remove();
    QTRY_COMPARE(model.rowCount(), prevCount);
}

void tst_AccountModel::updateAccount()
{
    AccountModel model;
    int prevCount = model.rowCount();

    QScopedPointer<AccountInterface> account(new AccountInterface);
    account->classBegin();
    account->setProviderName("test-provider");
    account->sync();
    account->componentComplete();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    QCOMPARE(model.rowCount(), prevCount+1);

    QSignalSpy spyDataChanged(&model, SIGNAL(dataChanged(QModelIndex,QModelIndex)));

    account->setDisplayName("blah blah");
    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    QTRY_COMPARE(spyDataChanged.count(), 1);
    QCOMPARE(model.data(model.index(0), AccountModel::AccountDisplayNameRole).toString(), account->displayName());

    QVERIFY(account->enabled());
    account->setEnabled(true);
    account->sync();
    QTRY_COMPARE(account->status(), AccountInterface::Synced);
    QTRY_COMPARE(spyDataChanged.count(), 2);
    QVERIFY(model.data(model.index(0), AccountModel::AccountEnabledRole).toBool());

    account->remove();
}

#include "tst_accountmodel.moc"
QTEST_MAIN(tst_AccountModel)
