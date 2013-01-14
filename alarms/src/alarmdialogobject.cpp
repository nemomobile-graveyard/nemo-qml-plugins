/*
 * Copyright (C) 2013 Jolla Ltd.
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

#include "alarmdialogobject.h"
#include <timed-voland/reminder>
#include <timed/interface>
#include <QDBusPendingReply>
#include <QDBusPendingCallWatcher>

extern Maemo::Timed::Interface *timed;

AlarmDialogObject::AlarmDialogObject(QObject *parent)
    : AlarmObject(parent), m_hideSnooze(false), m_hideDismiss(false), m_missed(false)
{
}

AlarmDialogObject::AlarmDialogObject(const Maemo::Timed::Voland::Reminder &data, QObject *parent)
    : AlarmObject(data.attributes(), parent),
      m_hideSnooze(data.hideSnoozeButton1()),
      m_hideDismiss(data.hideCancelButton2()),
      m_missed(data.isMissed())
{
    // Reminders mysteriously do not contain the 'COOKIE' attribute. Set it here.
    m_cookie = data.cookie();
}

void AlarmDialogObject::snooze()
{
    sendResponse(-1);
}

void AlarmDialogObject::dismiss()
{
    sendResponse(-2);
}

void AlarmDialogObject::close()
{
    sendResponse(0);
}

void AlarmDialogObject::closedExternally()
{
    // Closed by external forces, don't send a response, just emit
    emit closed(this);
}

void AlarmDialogObject::sendResponse(int code)
{
    QDBusPendingCallWatcher *w = new QDBusPendingCallWatcher(timed->dialog_response_async(id(), code), this);
    connect(w, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(responseReply(QDBusPendingCallWatcher*)));

    // Close dialog
    emit closed(this);
}

void AlarmDialogObject::responseReply(QDBusPendingCallWatcher *w)
{
    QDBusPendingReply<bool> reply = *w;
    w->deleteLater();

    if (reply.isError())
        qWarning() << "org.nemomobile.alarms: Error from sending alarm dialog response:" << reply.error();
}

