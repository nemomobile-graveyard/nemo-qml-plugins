/*
 * Copyright (C) 2013 Jolla Mobile <andrew.den.exter@jollamobile.com>
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

#include "seasideversitexport.h"

#include "seasidecache.h"

#include <QDeclarativeInfo>

#include <QVersitContactExporter>

#include <QScriptValue>
#include <QScriptValueIterator>

SeasideVersitExport::SeasideVersitExport(QObject *parent)
    : QObject(parent)
    , m_status(Null)
{
    SeasideCache::reference();

    connect(&m_writer, SIGNAL(stateChanged(QVersitWriter::State)),
            this, SLOT(writerStateChanged(QVersitWriter::State)));
}

SeasideVersitExport::~SeasideVersitExport()
{
    SeasideCache::cancelExport(this);
    SeasideCache::release();
}

QUrl SeasideVersitExport::target() const
{
    return m_target;
}

void SeasideVersitExport::setTarget(const QUrl &target)
{
    if (!target.isEmpty() && !target.isLocalFile()) {
        qmlInfo(this) << "Contacts can only be exported to local files";
    } else if (m_target != target) {
        m_target = target;
        emit targetChanged();
    }
}

SeasideVersitExport::Status SeasideVersitExport::status() const
{
    return m_status;
}

int SeasideVersitExport::count() const
{
    return m_count;
}

void SeasideVersitExport::exportContacts()
{
    exportContacts(QScriptValue());
}

void SeasideVersitExport::exportContacts(const QScriptValue &contactIds)
{
    if (m_status == Active) {
        qmlInfo(this) << "An export is already in progress";
        return;
    }

    if (m_target.isEmpty())
        return;

    m_contactIds.clear();
    if (contactIds.isNumber()) {
        m_contactIds.append(contactIds.toUInt32());
    } else if (contactIds.isArray()) {
        QScriptValueIterator iterator(contactIds);
        while (iterator.hasNext()) {
            iterator.next();
            const QScriptValue contactId = iterator.value();
            if (contactId.isNumber())
                m_contactIds.append(contactId.toUInt32());
        }
        if (m_contactIds.isEmpty())
            return;
    }

    m_activeTarget = m_target;
    m_status = Active;

    SeasideCache::populateCache(this, m_contactIds);

    emit statusChanged();
}

void SeasideVersitExport::cancel()
{
    if (m_writer.state() == QVersitWriter::ActiveState) {
        m_writer.cancel();
    } else if (m_status == Active) {
        SeasideCache::cancelExport(this);
    } else {
        return;
    }

    m_status = Finished;
    emit statusChanged();
}

void SeasideVersitExport::cachePopulated()
{
    m_output.setFileName(m_activeTarget.toLocalFile());
    if (!m_output.open(QIODevice::WriteOnly)) {
        qmlInfo(this) << "Output file is not writable";
        m_status = Error;
        emit statusChanged();
        return;
    }

    // Thread this.
    QVersitContactExporter exporter;
    if (!exporter.exportContacts(SeasideCache::contacts(m_contactIds))) {
        m_status = Error;
        emit statusChanged();
        return;
    }

    m_writer.setDevice(&m_output);
    m_writer.startWriting(exporter.documents());
}

void SeasideVersitExport::writerStateChanged(QVersitWriter::State state)
{
    if (state == QVersitWriter::FinishedState) {
        m_output.close();
        if (m_status == Active) {
            m_status = Finished;
            emit statusChanged();
        }
    } else if (state == QVersitWriter::CanceledState) {
        m_output.close();
    }
}
