/*
 * Copyright (C) 2013 Jolla Mobile <bea.lam@jollamobile.com>
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

#include "seasidenamegroupmodel.h"
#include <QDebug>

SeasideNameGroupModel::SeasideNameGroupModel(QObject *parent)
    : QAbstractListModel(parent)
{
    SeasideCache::registerNameGroupChangeListener(this);

    QHash<int, QByteArray> roles;
    roles.insert(NameRole, "name");
    roles.insert(EntryCount, "entryCount");
    setRoleNames(roles);

    QList<QChar> allGroups = SeasideCache::allNameGroups();
    QHash<QChar, int> existingGroups = SeasideCache::nameGroupCounts();
    if (!existingGroups.isEmpty()) {
        for (int i=0; i<allGroups.count(); i++)
            m_groups << SeasideNameGroup(allGroups[i], existingGroups.value(allGroups[i], 0));
    } else {
        for (int i=0; i<allGroups.count(); i++)
            m_groups << SeasideNameGroup(allGroups[i], 0);
    }
}

SeasideNameGroupModel::~SeasideNameGroupModel()
{
    SeasideCache::unregisterNameGroupChangeListener(this);
}

int SeasideNameGroupModel::rowCount(const QModelIndex &) const
{
    return m_groups.count();
}

QVariant SeasideNameGroupModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
        case NameRole:
            return QString(m_groups[index.row()].name);
        case EntryCount:
            return m_groups[index.row()].count;
    }
    return QVariant();
}

void SeasideNameGroupModel::nameGroupsUpdated(const QHash<QChar, int> &groups)
{
    if (groups.isEmpty())
        return;

    bool wasEmpty = m_groups.isEmpty();
    if (wasEmpty) {
        QList<QChar> allGroups = SeasideCache::allNameGroups();
        beginInsertRows(QModelIndex(), 0, allGroups.count() - 1);
        for (int i=0; i<allGroups.count(); i++)
            m_groups << SeasideNameGroup(allGroups[i], 0);
    }

    for (QHash<QChar,int>::const_iterator it = groups.begin(); it != groups.end(); ++it) {
        int index = m_groups.indexOf(SeasideNameGroup(it.key()));
        if (index < 0) {
            qWarning() << "SeasideNameGroupModel: no match for group" << it.key();
            continue;
        }
        m_groups[index].count = it.value();
        if (!wasEmpty)
            emit dataChanged(createIndex(index, 0), createIndex(index, 0));
    }

    if (wasEmpty) {
        endInsertRows();
        emit countChanged();
    }
}
