/*
 * Copyright (C) 2012 Jolla Ltd.
 * Contact: Martin Jones <martin.jones@jollamobile.com>
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
 *   * Neither the name of Jolla Ltd. nor the names of its contributors
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
#include <QDateTime>
#include <QtTest>

#include <QDeclarativeEngine>
#include <QDeclarativeComponent>

// Kind of tricky to test much of this without messing with system setttings.

class tst_WallClock: public QObject
{
    Q_OBJECT

private slots:
    void update();
};

void tst_WallClock::update()
{
    QDeclarativeEngine engine;
    QDeclarativeComponent comp(&engine, QUrl::fromLocalFile("/opt/tests/nemo-qml-plugins/time/update.qml"));
    QObject *obj = comp.create();
    QVERIFY(obj);

    QSignalSpy updateSpy(obj, SIGNAL(timeChanged()));

    QDateTime currentTime = QDateTime::currentDateTime();
    int year = obj->property("year").toInt();
    int month = obj->property("month").toInt();
    int day = obj->property("day").toInt();
    int hour = obj->property("hour").toInt();
    int minute = obj->property("minute").toInt();
    int second = obj->property("second").toInt();
    QCOMPARE(year, currentTime.date().year());
    QCOMPARE(month, currentTime.date().month()-1);
    QCOMPARE(day, currentTime.date().day());
    QCOMPARE(hour, currentTime.time().hour());
    QCOMPARE(minute, currentTime.time().minute());
    QCOMPARE(second, currentTime.time().second());
    int lastSecond = second;

    int upCount = updateSpy.count();
    int tryCount = 10;
    while (tryCount) {
        if (updateSpy.count() == upCount + 1)
            break;
        --tryCount;
        QTest::qWait(100);
    }
    QVERIFY(updateSpy.count() == upCount + 1);

    currentTime = QDateTime::currentDateTime();
    year = obj->property("year").toInt();
    month = obj->property("month").toInt();
    day = obj->property("day").toInt();
    hour = obj->property("hour").toInt();
    minute = obj->property("minute").toInt();
    second = obj->property("second").toInt();
    QCOMPARE(year, currentTime.date().year());
    QCOMPARE(month, currentTime.date().month()-1);
    QCOMPARE(day, currentTime.date().day());
    QCOMPARE(hour, currentTime.time().hour());
    QCOMPARE(minute, currentTime.time().minute());
    QCOMPARE(second, currentTime.time().second());

    QVERIFY((lastSecond+1)%60 == second);

    // would be nice to also test minute and day updates,
    // but who wants to wait 24hrs for a test to run.

    // disable and ensure updates don't happen.
    obj->setProperty("enabled", QVariant(false));

    upCount = updateSpy.count();
    QTest::qWait(1100);
    QVERIFY(updateSpy.count() == upCount);
}


#include "tst_wallclock.moc"
QTEST_MAIN(tst_WallClock)
