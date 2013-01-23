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

#ifndef ALARMOBJECT_H
#define ALARMOBJECT_H

#include <QObject>
#include <QDateTime>
#include <QMap>

class AlarmPrivate;
class QDBusPendingCallWatcher;

class AlarmObject : public QObject
{
    Q_OBJECT

public:
    AlarmObject(QObject *parent = 0);
    AlarmObject(const QMap<QString,QString> &data, QObject *parent = 0);

    /*!
     *  \qmlproperty string Alarm::title
     *
     *  User-specified title of the alarm. May be empty.
     */
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    QString title() const { return m_title; }
    void setTitle(const QString &title);

    /*!
     *  \qmlproperty int Alarm::hour
     *  Hour component of the alarm time
     *
     *  An alarm will trigger when the device time is next at the specified
     *  hour and minute, after being enabled. Alarm time is independent of
     *  timezones.
     */
    Q_PROPERTY(int hour READ hour WRITE setHour NOTIFY timeChanged)
    int hour() const { return m_hour; }
    void setHour(int hour);

    /*!
     *  \qmlproperty int Alarm::minute
     *  Minute component of the alarm time
     *
     *  An alarm will trigger when the device time is next at the specified
     *  hour and minute, after being enabled. Alarm time is independent of
     *  timezones.
     */
    Q_PROPERTY(int minute READ minute WRITE setMinute NOTIFY timeChanged)
    int minute() const { return m_minute; }
    void setMinute(int minute);

    /*!
     *  \qmlproperty string Alarm::daysOfWeek
     *  List of weekdays when the alarm will be repeated
     *  
     *  String containing the characters 'mtwTfsS' representing Monday through Sunday.
     *  If non-empty, the alarm will only trigger on these days, and will repeat until
     *  disabled.
     *
     *  By default (when this property is empty), alarms are single-shot: they trigger
     *  at the specified time in the next day, and then are disabled and won't trigger
     *  again unless enabled again.
     */
    Q_PROPERTY(QString daysOfWeek READ daysOfWeek WRITE setDaysOfWeek NOTIFY daysOfWeekChanged)
    QString daysOfWeek() const { return m_daysOfWeek; }
    void setDaysOfWeek(const QString &days);

    /*!
     *  \qmlproperty bool Alarm::enabled
     *
     *  True to enable and schedule the alarm, false when the alarm should not be active.
     */
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled);

    /*!
     *  \qmlproperty int Alarm::id
     *
     *  Internal identifier for the alarm. May change after modifications are saved.
     *  0 is an invalid ID, used for unsaved alarms.
     */
    Q_PROPERTY(int id READ id NOTIFY idChanged);
    int id() const { return static_cast<int>(m_cookie); }

    /*!
     *  \qmlproperty datetime Alarm::createdDate
     *
     *  Date when the alarm was first created
     */
    Q_PROPERTY(QDateTime createdDate READ createdDate CONSTANT);
    QDateTime createdDate() const { return m_createdDate; }

    /*!
     *  \qmlmethod void Alarm::save()
     *
     *  Commit changes to the object to the backend. No modifications, including \a enabled,
     *  take effect until this method is called.
     *  
     *  \sa updated, saved
     */
    Q_INVOKABLE void save();

    /*!
     *  \qmlmethod void Alarm::deleteAlarm()
     *
     *  Remove this alarm from the backend.
     */
    Q_INVOKABLE void deleteAlarm();

signals:
    void titleChanged();
    void timeChanged();
    void daysOfWeekChanged();
    void enabledChanged();
    void idChanged();

    /*!
     *  \qmlsignal Alarm::updated()
     *
     *  Emitted at the beginning of a save operation, indicating that the data is updated.
     *
     *  \sa saved
     */
    void updated();
    /*!
     *  \qmlsignal Alarm::saved()
     *
     *  Emitted when changes have been recorded to the backend.
     *
     *  \sa updated
     */
    void saved();
    /*!
     *  \qmlsignal Alarm::deleted()
     *
     *  Emitted after a call to deleteAlarm(), indicating that this object should be removed
     *  and no longer used.
     *
     *  \sa deleteAlarm
     */
    void deleted();

private slots:
    void saveReply(QDBusPendingCallWatcher *w);
    void deleteReply(QDBusPendingCallWatcher *w);

protected:
    QString m_title;
    int m_hour, m_minute;
    QString m_daysOfWeek;
    bool m_enabled;
    QDateTime m_createdDate;

    // Timed
    unsigned m_cookie;
};

#endif

