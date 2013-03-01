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

#ifndef SEASIDEVERSITEXPORT_H
#define SEASIDEVERSITEXPORT_H

#include <qdeclarative.h>

#include <QFile>
#include <QUrl>

#include <QContactId>

#include <QVersitWriter>

QT_BEGIN_NAMESPACE
class QScriptValue;
QT_END_NAMESPACE

QTM_USE_NAMESPACE

class SeasideVersitExport : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl target READ target WRITE setTarget NOTIFY targetChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_ENUMS(Status)
public:
    enum Status
    {
        Null,
        Active,
        Finished,
        Error
    };

    SeasideVersitExport(QObject *parent = 0);
    ~SeasideVersitExport();

    QUrl target() const;
    void setTarget(const QUrl &target);

    Status status() const;

    int count() const;

    // For SeasideCache
    void cachePopulated();

public slots:
    void exportContacts();
    void exportContacts(const QScriptValue &contactIds);
    void cancel();

signals:
    void finished();
    void targetChanged();
    void statusChanged();
    void countChanged();

private slots:
    void writerStateChanged(QVersitWriter::State state);

private:
    QVersitWriter m_writer;
    QFile m_output;
    QUrl m_target;
    QUrl m_activeTarget;
    QList<QContactLocalId> m_contactIds;
    int m_count;
    Status m_status;
};

QML_DECLARE_TYPE(SeasideVersitExport)

#endif
