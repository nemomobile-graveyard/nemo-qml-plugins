/*
 * Copyright (C) 2012 Jolla Ltd. <john.brooks@jollamobile.com>
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

#include "alarmsbackendmodel.h"
#include "alarmobject.h"

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

class tst_AlarmsBackendModel : public QObject
{
    Q_OBJECT

private slots:
    void populated();
    void createAndDelete();
    void setAlarmProperties();
};

void tst_AlarmsBackendModel::populated()
{
    QScopedPointer<AlarmsBackendModel> model(new AlarmsBackendModel);
    QCOMPARE(model->isPopulated(), false);
    QSignalSpy spy(model.data(), SIGNAL(populatedChanged()));
    QTRY_COMPARE(model->isPopulated(), true);
    QCOMPARE(spy.count(), 1);
}

void tst_AlarmsBackendModel::createAndDelete()
{
    QScopedPointer<AlarmsBackendModel> model(new AlarmsBackendModel);
    QTRY_COMPARE(model->isPopulated(), true);

    int oldRowCount = model->rowCount();
    
    AlarmObject *alarm = model->createAlarm();
    alarm->setTitle(QLatin1String("Test Alarm"));
    alarm->setHour(10);
    alarm->setMinute(30);
    alarm->setDaysOfWeek(QLatin1String("mwf"));
    QVERIFY(!alarm->isEnabled());
    QCOMPARE(alarm->id(), 0);
    alarm->save();

    // Alarm should be immediately available in the model
    QCOMPARE(model->rowCount(), oldRowCount + 1);
 
    // Alarm will be asynchronously saved and updated with the backend ID
    QTRY_VERIFY(alarm->id() > 0);
    int alarmId = alarm->id();

    // Find position
    int alarmRow = -1;
    for (int i = 0; i < model->rowCount(); i++) {
        alarm = qobject_cast<AlarmObject*>(model->data(model->index(i, 0),
                    AlarmsBackendModel::AlarmObjectRole).value<QObject*>());
        if (alarm && alarm->id() == alarmId) {
            alarmRow = i;
            break;
        }
    }
    QVERIFY(alarmRow >= 0);

    // Object will be freed when the model is destroyed
    {
        QSignalSpy spy(alarm, SIGNAL(destroyed()));
        model.reset(new AlarmsBackendModel);
        alarm = 0;
        QCOMPARE(spy.count(), 1);
    }

    // Wait to populate
    QTRY_COMPARE(model->isPopulated(), true);

    // Alarm should be in the same position in the model after reloading
    alarm = qobject_cast<AlarmObject*>(model->data(model->index(alarmRow, 0),
                AlarmsBackendModel::AlarmObjectRole).value<QObject*>());
    QVERIFY(alarm);
    QCOMPARE(alarm->id(), alarmId);

    QCOMPARE(alarm->title(), QLatin1String("Test Alarm"));
    QCOMPARE(alarm->hour(), 10);
    QCOMPARE(alarm->minute(), 30);
    QCOMPARE(alarm->daysOfWeek(), QLatin1String("mwf"));
    QCOMPARE(alarm->isEnabled(), false);

    // Enable and save
    {
        alarm->setEnabled(true);
        QCOMPARE(alarm->isEnabled(), true);

        QSignalSpy spy(alarm, SIGNAL(saved()));
        alarm->save();
        QTRY_COMPARE(spy.count(), 1);
    }

    // Removed from model immediately
    alarm->deleteAlarm();
    QCOMPARE(model->rowCount(), oldRowCount);
}

void tst_AlarmsBackendModel::setAlarmProperties()
{
    QScopedPointer<AlarmsBackendModel> model(new AlarmsBackendModel);

    AlarmObject *alarm = model->createAlarm();

    {
        QSignalSpy spy(alarm, SIGNAL(titleChanged()));
        alarm->setTitle(QLatin1String("Test Alarm"));
        QCOMPARE(spy.count(), 1);
    }

    {
        QSignalSpy spy(alarm, SIGNAL(timeChanged()));
        alarm->setHour(10);
        alarm->setMinute(30);
        QCOMPARE(spy.count(), 2);
    }

    {
        QSignalSpy spy(alarm, SIGNAL(daysOfWeekChanged()));
        alarm->setDaysOfWeek("mfwS");
        QCOMPARE(spy.count(), 1);
    }

    {
        QSignalSpy spy(alarm, SIGNAL(enabledChanged()));
        alarm->setEnabled(true);
        QCOMPARE(spy.count(), 1);
    }

    alarm->deleteAlarm();
}

#include "tst_alarmsbackendmodel.moc"
QTEST_MAIN(tst_AlarmsBackendModel)
