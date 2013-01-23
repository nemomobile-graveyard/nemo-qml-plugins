/*
 * Copyright (C) 2012 Jolla Ltd.
 * Contact: John Brooks <john.brooks@jollamobile.com>
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

#include "alarmobject.h"
#include "interface.h"
#include <QDBusPendingReply>
#include <timed/event>
#include <timed/exception>

AlarmObject::AlarmObject(QObject *parent)
    : QObject(parent), m_hour(0), m_minute(0), m_enabled(false),
      m_createdDate(QDateTime::currentDateTime()), m_cookie(0)
{
}

AlarmObject::AlarmObject(const QMap<QString,QString> &data, QObject *parent)
    : QObject(parent), m_hour(0), m_minute(0), m_enabled(false),
      m_createdDate(QDateTime::currentDateTime()), m_cookie(0)
{
    for (QMap<QString,QString>::ConstIterator it = data.begin(); it != data.end(); it++) {
        if (it.key() == "TITLE")
            setTitle(it.value());
        else if (it.key() == "COOKIE")
            m_cookie = it.value().toUInt();
        else if (it.key() == "daysOfWeek")
            setDaysOfWeek(it.value());
        else if (it.key() == "createdDate")
            m_createdDate = QDateTime::fromString(it.value());
        else if (it.key() == "timeOfDay") {
            int value = it.value().toInt();
            m_hour = value / 60;
            m_minute = value % 60;
        } else if (it.key() == "STATE") {
            if (it.value() == "TRANQUIL" || it.value() == "WAITING")
                m_enabled = false;
            else
                m_enabled = true;
        }
    }
}

void AlarmObject::setTitle(const QString &t)
{
    if (m_title == t)
        return;

    m_title = t;
    emit titleChanged();
}

void AlarmObject::setHour(int hour)
{
    if (m_hour == hour)
        return;

    m_hour = hour;
    emit timeChanged();
}

void AlarmObject::setMinute(int minute)
{
    if (m_minute == minute)
        return;

    m_minute = minute;
    emit timeChanged();
}

void AlarmObject::setDaysOfWeek(const QString &in) 
{
    QString str;
    for (int i = 0; i < in.size(); i++) {
        switch (in[i].toLatin1()) {
            case 'm':
            case 't':
            case 'w':
            case 'T':
            case 'f':
            case 's':
            case 'S':
                str += in[i];
                break;
            default:
                qWarning() << Q_FUNC_INFO << "Invalid input string:" << in;
                return;
        }
    }

    m_daysOfWeek = str;
    emit daysOfWeekChanged();
}

void AlarmObject::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;

    m_enabled = enabled;
    emit enabledChanged();
}

void AlarmObject::save()
{
    try {
        Maemo::Timed::Event ev;
        // Keep the event after it has triggered
        ev.setKeepAliveFlag();
        ev.setAlarmFlag();
        // Trigger the voland alarm/reminder dialog
        ev.setReminderFlag();

        ev.setAttribute(QLatin1String("APPLICATION"), QLatin1String("nemoalarms"));
        ev.setAttribute(QLatin1String("createdDate"), m_createdDate.toString());
        if (!m_title.isEmpty())
            ev.setAttribute(QLatin1String("TITLE"), m_title);

        ev.setAttribute(QLatin1String("timeOfDay"), QString::number(m_hour * 60 + m_minute));

        if (!m_daysOfWeek.isEmpty())
            ev.setAttribute(QLatin1String("daysOfWeek"), m_daysOfWeek);

        if (m_enabled) {
            Maemo::Timed::Event::Recurrence rec = ev.addRecurrence();
            rec.addHour(m_hour);
            rec.addMinute(m_minute);
            rec.everyDayOfMonth();
            rec.everyMonth();

            // Single-shot alarms are done with a recurrence and the single-shot
            // flag, which removes recurrence information after the first trigger.
            if (m_daysOfWeek.isEmpty()) {
                rec.everyDayOfWeek();
                ev.setSingleShotFlag();
            }

            // Map characters to numeric weekdays used by libtimed
            QString weekdayMap(QLatin1String("SmtwTfs"));
            for (int i = 0; i < m_daysOfWeek.size(); i++) {
                int day = weekdayMap.indexOf(m_daysOfWeek[i]);
                if (day >= 0)
                    rec.addDayOfWeek(day);
            }
        }

        QDBusPendingCallWatcher *w;
        if (m_cookie)
            w = new QDBusPendingCallWatcher(TimedInterface::instance()->replace_event_async(ev, m_cookie), this);
        else
            w = new QDBusPendingCallWatcher(TimedInterface::instance()->add_event_async(ev), this);
        connect(w, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(saveReply(QDBusPendingCallWatcher*)));

        // Emit the updated signal immediately to update UI
        emit updated();
    } catch (Maemo::Timed::Exception &e) {
        qWarning() << "org.nemomobile.alarms: Cannot sync alarm to timed:" << e.what();
    }
}

void AlarmObject::saveReply(QDBusPendingCallWatcher *w)
{
    QDBusPendingReply<uint> reply = *w;
    w->deleteLater();

    if (reply.isError()) {
        qWarning() << "org.nemomobile.alarms: Cannot sync alarm to timed:" << reply.error();
        return;
    }

    m_cookie = reply.value();
    emit idChanged();
    emit saved();
}

void AlarmObject::deleteAlarm()
{
    if (!m_cookie) {
        emit deleted();
        return;
    }

    QDBusPendingCall re = TimedInterface::instance()->cancel_async(m_cookie);
    QDBusPendingCallWatcher *w = new QDBusPendingCallWatcher(re, this);
    connect(w, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(deleteReply(QDBusPendingCallWatcher*)));

    emit deleted();
    m_cookie = 0;
    emit idChanged();
}

void AlarmObject::deleteReply(QDBusPendingCallWatcher *w)
{
    QDBusPendingReply<bool> reply = *w;
    w->deleteLater();

    if (reply.isError())
        qWarning() << "org.nemomobile.alarms: Cannot delete alarm from timed:" << reply.error();
}

