/*
 * Copyright (C) 2013 Jolla Ltd. <john.brooks@jollamobile.com>
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
#include <QDBusAbstractAdaptor>
#include <QDBusInterface>
#include <QDBusArgument>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusMetaType>

#include "alarmhandlerinterface.h"
#include "alarmdialogobject.h"

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

class tst_AlarmHandler : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void openDialog();
};

struct TestButtons {
    QMap<QString, QString> attr;
};
Q_DECLARE_METATYPE(TestButtons);

QDBusArgument &operator<<(QDBusArgument &out, const TestButtons &x)
{
    out.beginStructure();
    out << x.attr;
    out.endStructure();
    return out;
}

const QDBusArgument &operator>>(const QDBusArgument &in, TestButtons &x)
{
    in.beginStructure();
    in >> x.attr;
    in.endStructure();
    return in;
}

struct TestReminder {
    quint32 cookie, flags;
    QMap<QString, QString> attr;
    QVector<TestButtons> buttons;

    TestReminder()
        : cookie(1), flags(0)
    {
        attr["TITLE"] = "Test Alarm";
        attr["APPLICATION"] = "nemoalarms";
        attr["timeOfDay"] = "360";
    }
};
Q_DECLARE_METATYPE(TestReminder);

QDBusArgument &operator<<(QDBusArgument &out, const TestReminder &x)
{
    out.beginStructure();
    out << x.cookie << x.flags << x.attr << x.buttons;
    out.endStructure();
    return out;
}

const QDBusArgument &operator>>(const QDBusArgument &in, TestReminder &x)
{
    in.beginStructure();
    in >> x.cookie >> x.flags >> x.attr >> x.buttons;
    in.endStructure();
    return in;
}

Q_DECLARE_METATYPE(QDBusPendingCallWatcher*);

void tst_AlarmHandler::initTestCase()
{
    qDBusRegisterMetaType<TestButtons>();
    qDBusRegisterMetaType<TestReminder>();
    qRegisterMetaType<QDBusPendingCallWatcher*>();
}

void tst_AlarmHandler::openDialog()
{
    QScopedPointer<AlarmHandlerInterface> handler(new AlarmHandlerInterface);

    {
        QSignalSpy spy(handler.data(), SIGNAL(error(QString)));
        QTest::qWait(0);
        QCOMPARE(spy.count(), 0);
    }

    QScopedPointer<QDBusInterface> interface(new QDBusInterface("org.nemomobile.alarms.test.voland", "/com/nokia/voland"));
    QVERIFY(interface->isValid());

    QVERIFY(handler->activeDialogs().isEmpty());

    AlarmDialogObject *alarm = 0;
    QSignalSpy activeDialogsSpy(handler.data(), SIGNAL(activeDialogsChanged()));

    {
        QSignalSpy spy(handler.data(), SIGNAL(alarmReady(QObject*)));

        TestReminder reminder;
        QVariantList list;
        list << QVariant::fromValue<TestReminder>(reminder);
        QDBusPendingCall call = interface->asyncCallWithArgumentList("open", list);
        QScopedPointer<QDBusPendingCallWatcher> w(new QDBusPendingCallWatcher(call, interface.data()));
 
        QSignalSpy callSpy(w.data(), SIGNAL(finished(QDBusPendingCallWatcher*)));
        QTRY_COMPARE(callSpy.count(), 1);
 
        QDBusPendingReply<bool> reply = *w.data();
        if (reply.isError()) {
            qDebug() << reply.error();
            QVERIFY(!reply.isError());
        }
        QVERIFY(reply.value());

        QCOMPARE(spy.count(), 1);
        QCOMPARE(activeDialogsSpy.count(), 1);
        QCOMPARE(handler->activeDialogs().size(), 1);
        alarm = qobject_cast<AlarmDialogObject*>(spy.takeFirst().at(0).value<QObject*>());
    }

    QVERIFY(alarm);
    QCOMPARE(alarm->id(), 1);
    QCOMPARE(alarm->title(), QString("Test Alarm"));

    QSignalSpy spy(alarm, SIGNAL(closed(QObject*)));
    alarm->dismiss();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(activeDialogsSpy.count(), 2);
    QVERIFY(handler->activeDialogs().isEmpty());
}

#include "tst_alarmhandler.moc"
QTEST_MAIN(tst_AlarmHandler)
