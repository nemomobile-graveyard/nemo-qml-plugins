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

#include <QTime>
#include <QPauseAnimation>
#include <QDebug>
#include "nemowallclock.h"
#include "nemowallclock_p.h"

extern WallClockPrivate *nemoCreateWallClockPrivate(WallClock *wc);

WallClockPrivate::WallClockPrivate(WallClock *wallClock)
    : q(wallClock), m_updateFreq(WallClock::Second), m_enabled(true)
{
    setLoopCount(-1);
    update();
}

WallClockPrivate::~WallClockPrivate()
{
}

void WallClockPrivate::setUpdateFrequency(WallClock::UpdateFrequency freq)
{
    if (freq == m_updateFreq)
        return;

    m_updateFreq = freq;
    update();
    emit q->updateFrequencyChanged();
}

void WallClockPrivate::setEnabled(bool e)
{
    if (m_enabled == e)
        return;

    m_enabled = e;
    update();
    emit q->enabledChanged();
}

QDateTime WallClockPrivate::time() const
{
    return QDateTime::currentDateTime();
}

QString WallClockPrivate::timezone() const
{
    return QString();
}

QString WallClockPrivate::timezoneAbbreviation() const
{
    return QString();
}

void WallClockPrivate::update() {
    if (m_enabled) {
        QTime current = QTime::currentTime();
        int initDelay = 0;
        switch (m_updateFreq) {
        case WallClock::Day:
            initDelay = (23 - current.hour()) * 3600 * 1000;
            initDelay += (59 - current.minute()) * 60 * 1000;
            // fall through
        case WallClock::Minute:
            initDelay += (59 - current.second()) * 1000;
            // fall through
        case WallClock::Second:
            initDelay += 1000 - current.msec();
        }

        setCurrentTime(0);
        setDuration(initDelay+8); // animation timer can fire slightly before our target
        if (state() != Running)
            start();
    } else {
        stop();
    }
}

void WallClockPrivate::updateCurrentTime(int)
{
    if (currentLoop() == 0 || currentTime() == 0)
        return;

    emit q->timeChanged();

    update();
}

void WallClockPrivate::timezoneChanged()
{
    emit q->timezoneChanged();
}

void WallClockPrivate::timezoneAbbreviationChanged()
{
    emit q->timezoneAbbreviationChanged();
}

void WallClockPrivate::systemTimeChanged()
{
    emit q->systemTimeUpdated();
}

void WallClockPrivate::timeChanged()
{
    emit q->timeChanged();
}

//===========================================================================

/*!
    \qmlclass WallClock WallClock
    \brief The WallClock type provides the current date/time

    import org.nemomobile.time 1.0

    The WallClock type provides the current date and time via
    the \l time JS Date property.

    The time property may be updated on the change of the day, minute,
    or second depending on the \l updateFrequency property.
*/

/*!
    \qmlsignal WallClock::onSystemTimeUpdated

    This handler is called when the system time changes.
*/
WallClock::WallClock(QObject *parent)
    : QObject(parent), d(nemoCreateWallClockPrivate(this))
{
}

WallClock::~WallClock()
{
    delete d;
}

/*!
    \qmlproperty bool WallClock::enabled

    Sets whether the \l time is updated periodically as determined by the
    \l updateFrequency property.
*/
bool WallClock::enabled() const
{
    return d->enabled();
}

void WallClock::setEnabled(bool enabled)
{
    d->setEnabled(enabled);
}

/*!
    \qmlproperty Date WallClock::time

    This property provides the current date and time as a JS Date object.

    \l time will be updated according to the \l updateFrequency.
*/
QDateTime WallClock::time() const
{
    return d->time();
}

/*!
    \qmlproperty string WallClock::timezone

    Provides the current timezone.  Depending upon the user's settings
    this may be either a location such as
    Australia/Brisbane, or an offset from UTC such as GMT+10:00.
*/
QString WallClock::timezone() const
{
    return d->timezone();
}

/*!
    \qmlproperty string WallClock::timezoneAbbreviation

    Provides the current timezone abbreviation, for example: 'EEST', 'AEST'.
*/
QString WallClock::timezoneAbbreviation() const
{
    return d->timezoneAbbreviation();
}

/*!
    \qmlproperty enumeration WallClock::updateFrequency

    \list
    \o WallClock.Day - notify on the change of day
    \o WallClock.Minute - notify on the change of minute
    \o WallClock.Second - notify on the change of the second
    \endlist
*/
WallClock::UpdateFrequency WallClock::updateFrequency() const
{
    return d->updateFrequency();
}

void WallClock::setUpdateFrequency(WallClock::UpdateFrequency freq)
{
    d->setUpdateFrequency(freq);
}
