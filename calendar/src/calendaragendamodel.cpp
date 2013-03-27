/*
 * Copyright (C) 2013 Jolla Ltd.
 * Contact: Robin Burchell <robin.burchell@jollamobile.com>
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

// Qt
#include <QDebug>

// mkcal
#include <event.h>

#include "calendareventcache.h"
#include "calendaragendamodel.h"
#include "calendarevent.h"
#include "calendardb.h"

NemoCalendarAgendaModel::NemoCalendarAgendaModel(QObject *parent)
    : NemoCalendarAbstractModel(parent)
{
    QHash<int,QByteArray> roles;
    roles[EventObjectRole] = "event";
    roles[SectionBucketRole] = "sectionBucket";
    roles[NotebookColorRole] = "notebookColor";
    setRoleNames(roles);

    connect(this, SIGNAL(startDateChanged()), this, SLOT(load()));
    connect(NemoCalendarEventCache::instance(), SIGNAL(modelReset()), this, SLOT(load()));
}

NemoCalendarAgendaModel::~NemoCalendarAgendaModel()
{
    qDeleteAll(mEvents);
}

QDate NemoCalendarAgendaModel::startDate() const
{
    return mStartDate;
}

void NemoCalendarAgendaModel::setStartDate(const QDate &startDate)
{
    if (mStartDate == startDate)
        return;

    mStartDate = startDate;
    emit startDateChanged();
}

void NemoCalendarAgendaModel::load()
{
    // TODO: we really need a centralised event cache
    KCalCore::Event::List eventList = NemoCalendarDb::calendar()->rawEventsForDate(mStartDate, KDateTime::Spec(KDateTime::LocalZone), KCalCore::EventSortStartDate, KCalCore::SortDirectionAscending);
    qDebug() << Q_FUNC_INFO << "Loaded " << eventList.count() << " events for " << mStartDate;

    beginResetModel();
    qDeleteAll(mEvents);
    mEvents.clear();
    mEvents.reserve(eventList.size());

    foreach (const KCalCore::Event::Ptr &evt, eventList) {
        NemoCalendarEvent *event = new NemoCalendarEvent(evt);
        mEvents.append(event);
    }

    endResetModel();
}

int NemoCalendarAgendaModel::rowCount(const QModelIndex &index) const
{
    if (index != QModelIndex())
        return 0;

    return mEvents.size();
}

QVariant NemoCalendarAgendaModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= mEvents.count())
        return QVariant();

    switch (role) {
        case EventObjectRole:
            return QVariant::fromValue<QObject *>(mEvents.at(index.row()));
        case SectionBucketRole:
            return mEvents.at(index.row())->startTime().date();
        case NotebookColorRole:
            return "#00aeef"; // TODO: hardcoded, as we only support local events for now
        default:
            return QVariant();
    }
}

