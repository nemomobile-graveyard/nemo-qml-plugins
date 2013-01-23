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

#ifndef ALARMHANDLERINTERFACE_H
#define ALARMHANDLERINTERFACE_H

#include <QDeclarativeItem>
#include <timed-voland/interface>

class VolandAdaptor;
class AlarmDialogObject;

namespace Maemo {
    namespace Timed {
        namespace Voland {
            class Reminder;
        }
    }
}

class AlarmHandlerInterface : public QDeclarativeItem
{
    Q_OBJECT

public:
    AlarmHandlerInterface(QDeclarativeItem *parent = 0);

signals:
    /*!
     *  \qmlsignal void AlarmHandler::alarmReady(AlarmDialog alarm)
     *
     *  Emitted when an alarm has triggered and should be displayed. The alarm
     *  object contains properties and actions for the alarm.
     */
    void alarmReady(QObject *alarm);
    
    /*!
     *  \qmlsignal void AlarmHandler::error(string message)
     *
     *  Emitted when the alarm handler interface cannot be registered.
     */
    void error(const QString &message);

private slots:
    void dialogClosed(QObject *alarm);
    void setupInterface();

private:
    VolandAdaptor *adaptor;
    QHash<int, AlarmDialogObject*> dialogs;

    friend class VolandAdaptor;

    AlarmDialogObject *createDialog(const Maemo::Timed::Voland::Reminder &data);
};

class VolandAdaptor : public Maemo::Timed::Voland::AbstractAdaptor
{
#ifdef USE_VOLAND_TEST_INTERFACE
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.nemomobile.alarms.test.voland");
#endif

public:
    AlarmHandlerInterface *q;

    VolandAdaptor(AlarmHandlerInterface *p)
        : Maemo::Timed::Voland::AbstractAdaptor(p), q(p)
    {
    }

    virtual bool open(const Maemo::Timed::Voland::Reminder &data);
    virtual bool open(const QList<QVariant> &data);
    virtual bool close(uint cookie);
};

#endif

