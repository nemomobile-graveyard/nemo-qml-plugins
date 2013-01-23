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

#ifndef ALARMDIALOGOBJECT_H
#define ALARMDIALOGOBJECT_H

#include "alarmobject.h"

namespace Maemo {
    namespace Timed {
        namespace Voland {
            class Reminder;
        }
    }
}

class QDBusPendingCallWatcher;

class AlarmDialogObject : public AlarmObject
{
    Q_OBJECT

    friend class AlarmHandlerInterface;

public:
    AlarmDialogObject(QObject *parent = 0);
    AlarmDialogObject(const Maemo::Timed::Voland::Reminder &data, QObject *parent = 0);

    /*!
     *  \qmlproperty AlarmDialog::hideSnoozeButton
     *
     *  Flag indicating that the snooze button should not be displayed for this dialog
     */
    Q_PROPERTY(bool hideSnoozeButton READ hideSnoozeButton CONSTANT)
    bool hideSnoozeButton() const { return m_hideSnooze; }

    /*!
     *  \qmlproperty AlarmDialog::hideDismissButton
     *
     *  Flag indicating that the dismiss button should not be displayed for this dialog
     */
    Q_PROPERTY(bool hideDismissButton READ hideDismissButton CONSTANT)
    bool hideDismissButton() const { return m_hideDismiss; }

    /*!
     *  \qmlproperty AlarmDialog::isMissed
     *
     *  Flag indicating that the time for this alarm was missed and fired late
     */
    Q_PROPERTY(bool isMissed READ isMissed CONSTANT)
    bool isMissed() const { return m_missed; }

    /*!
     *  \qmlmethod void AlarmDialog::snooze()
     *
     *  Snooze the alarm for the system's default snooze duration. The alarm will
     *  trigger again afterwards.
     */
    Q_INVOKABLE void snooze();
    /*!
     *  \qmlmethod void AlarmDialog::dismiss()
     *
     *  Dismiss the alarm. Repeatable alarms will be triggered again on the next repeat
     *  day, and non-repeatable alarms will be disabled.
     */
    Q_INVOKABLE void dismiss();
    /*!
     *  \qmlmethod void AlarmDialog::close()
     *
     *  Close the alarm dialog and use the default action for the alarm. This usually happens
     *  as a result of no interaction from the user.
     */
    Q_INVOKABLE void close();

    void closedExternally();

signals:
    /*!
     *  \qmlsignal void closed(AlarmDialog alarm)
     *
     *  Emitted when the dialog has been closed, either by sending a response or from
     *  an external action or timeout. UI should be closed in response.
     */
    void closed(QObject *alarm);

private slots:
    void responseReply(QDBusPendingCallWatcher *w);

private:
    bool m_hideSnooze, m_hideDismiss, m_missed;

    void sendResponse(int code);
};

#endif

