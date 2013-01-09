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

#include "serviceinterface.h"

#include "accountmanagerinterface.h"
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

class tst_ServiceInterface : public QObject
{
    Q_OBJECT

private slots:
    //properties
    void properties();
};

void tst_ServiceInterface::properties()
{
    QPointer<ServiceInterface> si;
    {
        QScopedPointer<AccountManagerInterface> m(new AccountManagerInterface);
        m->classBegin();
        m->componentComplete();
        ServiceInterface *srv = m->service("test-service2");
        QVERIFY(srv);
        QCOMPARE(srv->name(), QString(QLatin1String("test-service2")));
        QCOMPARE(srv->serviceType(), QString(QLatin1String("test-service-type2")));
        QCOMPARE(srv->displayName(), QString(QLatin1String("Test Service")));
        QCOMPARE(srv->providerName(), QString(QLatin1String("test-provider")));
        QCOMPARE(srv->iconName(), QString(QLatin1String("test")));
        QVERIFY(srv->tags().contains(QString(QLatin1String("testing"))));
        si = srv;
        QVERIFY(!si.isNull());
    }
    QTRY_VERIFY(si.isNull());
}

#include "tst_serviceinterface.moc"
QTEST_MAIN(tst_ServiceInterface)
