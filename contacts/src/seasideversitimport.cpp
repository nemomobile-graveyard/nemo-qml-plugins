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

#include "seasideversitimport.h"

#include "seasidecache.h"

#include <QDeclarativeInfo>

#include <QVersitContactImporter>

SeasideVersitImport::SeasideVersitImport(QObject *parent)
    : QObject(parent)
    , m_status(Null)
{
    SeasideCache::reference();

    connect(&m_reader, SIGNAL(stateChanged(QVersitReader::State)),
            this, SLOT(readerStateChanged(QVersitReader::State)));
}

SeasideVersitImport::~SeasideVersitImport()
{
    SeasideCache::cancelImport(this);
    SeasideCache::release();
}

QUrl SeasideVersitImport::source() const
{
    return m_source;
}

void SeasideVersitImport::setSource(const QUrl &source)
{
    // We could support network files with QNetworkAccessManager, but QVersitReader
    // accesses its input device directly from another thread
    if (!source.isEmpty() && !source.isLocalFile()) {
        qmlInfo(this) << "Contacts can only be exported from local files";
    } if (m_source != source) {
        m_source = source;
        emit sourceChanged();
    }
}

SeasideVersitImport::Status SeasideVersitImport::status() const
{
    return m_status;
}

int SeasideVersitImport::count() const
{
    return m_count;
}

void SeasideVersitImport::importContacts()
{
    if (m_status == Active) {
        qmlInfo(this) << "An import is already in progress";
        return;
    }

    if (m_source.isEmpty())
        return;

    m_status = Error;

    m_input.setFileName(m_source.toLocalFile());
    if (m_input.open(QIODevice::ReadOnly)) {
        m_reader.setDevice(&m_input);
        m_reader.startReading();

        if (m_reader.error() == QVersitReader::NoError)
            m_status = Active;
    } else {
        qmlInfo(this) << "Cannot write to file:" << m_source;
        qmlInfo(this) << m_input.errorString();
    }

    emit statusChanged();
}

void SeasideVersitImport::cancel()
{
    if (m_reader.state() == QVersitReader::ActiveState) {
        m_reader.cancel();
    } else if (m_status == Active) {
        SeasideCache::cancelImport(this);
    } else {
        return;
    }

    m_status = Finished;
    emit statusChanged();
}

void SeasideVersitImport::readerStateChanged(QVersitReader::State state)
{
    if (state == QVersitReader::FinishedState) {
        m_input.close();
        if (m_status == Active) {
            // TODO: This is slow enough to be threaded.
            QVersitContactImporter importer;
            importer.importDocuments(m_reader.results());

            const QList<QContact> contacts = importer.contacts();

            if (!contacts.isEmpty()) {
                SeasideCache::createContacts(contacts, this);

                m_count = contacts.count();
                emit countChanged();
            } else {
                m_status = Finished;
                emit statusChanged();
            }
        }
    } else if (state == QVersitReader::CanceledState) {
        m_input.close();
    }
}

void SeasideVersitImport::writeCompleted()
{
    m_status = Finished;
    emit statusChanged();
}
