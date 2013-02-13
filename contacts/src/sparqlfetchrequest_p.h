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

#ifndef SPARQLFETCHREQUEST_P_H
#define SPARQLFETCHREQUEST_P_H

#include <QThread>

#include <QContactAbstractRequest>

#include <QMutex>
#include <QWaitCondition>

/*
    This shouldn't be necessary but the QtContacts tracker backend is slow to execute queries,
    filtering on the favorites property doesn't work, and there's no support for sorting a
    QContactLocalIdFetchRequest.

    I'm sure those issues can be addressed with enough effort, but this will produce results much
    much sooner.
*/

QTM_USE_NAMESPACE

class QSparqlConnection;
class QSparqlResult;

class SparqlFetchRequest : public QThread
{
    Q_OBJECT
public:
    SparqlFetchRequest(QObject *parent = 0);
    ~SparqlFetchRequest();

    QContactAbstractRequest::State state() const;

    bool queryData() const;
    void setQueryData(bool query);

    bool favoritesOnly() const;
    void setFavoritesOnly(bool favorites);

    bool sortOnFirstName() const;
    void setSortOnFirstName(bool first);

    QList<QContactLocalId> ids() const;
    QList<QContact> contacts() const;

    void start();
    void cancel();
    void reset();

    bool event(QEvent *event);

signals:
    void resultsAvailable();
    void stateChanged(QContactAbstractRequest::State state);

protected:
    void run();

private:
    QContactAbstractRequest::State executeQuery(
            QSparqlConnection *connection, bool queryData, bool favoritesOnly, bool sortOnFirstName);
    QContactAbstractRequest::State readContacts(QSparqlResult *results);
    QContactAbstractRequest::State readContactIds(QSparqlResult *results);
    QWaitCondition m_condition;
    mutable QMutex m_mutex;
    QList<QContactLocalId> m_contactIds;
    QList<QContact> m_contacts;
    QContactAbstractRequest::State m_state;
    QContactAbstractRequest::State m_threadState;
    bool m_queryData;
    bool m_threadQueryData;
    bool m_favoritesOnly;
    bool m_threadFavoritesOnly;
    bool m_sortOnFirstName;
    bool m_threadSortOnFirstName;
    bool m_resultsAvailable;
};

#endif
