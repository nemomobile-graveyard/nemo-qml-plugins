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

#ifndef ALARMSBACKENDMODEL_H
#define ALARMSBACKENDMODEL_H

#include <QAbstractListModel>

class AlarmsBackendModelPriv;
class AlarmObject;

class AlarmsBackendModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum {
        AlarmObjectRole = Qt::UserRole,
        EnabledRole,
        HourRole,
        MinuteRole,
        WeekDaysRole
    };

    AlarmsBackendModel(QObject *parent = 0);
    virtual ~AlarmsBackendModel();

    /*!
     *  \qmlmethod Alarm AlarmsModel::createAlarm
     *
     *  Create a new alarm object. After setting properties, call the save()
     *  method of the object to commit it to the backend and model.
     *
     *  If the operation is aborted, call the deleteAlarm() method of the object.
     */
    Q_INVOKABLE AlarmObject *createAlarm();

    /*!
     *  \qmlproperty bool AlarmsModel::populated
     *
     *  True when the model has loaded all available alarms from the backend.
     *  The model may still be empty afterwards.
     */
    Q_PROPERTY(bool populated READ isPopulated NOTIFY populatedChanged)
    bool isPopulated() const;

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;

signals:
    void populatedChanged();

private:
    friend class AlarmsBackendModelPriv;
    AlarmsBackendModelPriv *priv;
};

#endif // ALARMSBACKENDMODEL_H
