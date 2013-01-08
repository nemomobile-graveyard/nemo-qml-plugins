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

#ifndef WALLCLOCK_H
#define WALLCLOCK_H

#include <QObject>
#include <QDateTime>


class WallClockPrivate;

class WallClock : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QDateTime time READ time NOTIFY timeChanged)
    Q_PROPERTY(QString timezone READ timezone NOTIFY timezoneChanged)
    Q_PROPERTY(QString timezoneAbbreviation READ timezoneAbbreviation NOTIFY timezoneAbbreviationChanged)
    Q_PROPERTY(UpdateFrequency updateFrequency READ updateFrequency WRITE setUpdateFrequency NOTIFY updateFrequencyChanged)
    Q_ENUMS(UpdateFrequency)

public:
    WallClock(QObject *parent = 0);
    ~WallClock();

    enum UpdateFrequency { Day, Minute, Second };

    bool enabled() const;
    void setEnabled(bool);

    QDateTime time() const;
    QString timezone() const;
    QString timezoneAbbreviation() const;

    UpdateFrequency updateFrequency() const;
    void setUpdateFrequency(UpdateFrequency);

private slots:
    void updateTimer();

signals:
    void enabledChanged();
    void timeChanged();
    void timezoneChanged();
    void timezoneAbbreviationChanged();
    void updateFrequencyChanged();
    void systemTimeUpdated();

private:
    friend class WallClockPrivate;
    WallClockPrivate *d;
};

#endif // WALLCLOCK_H
