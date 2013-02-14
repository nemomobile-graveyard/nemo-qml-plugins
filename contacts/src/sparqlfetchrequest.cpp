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

#include "sparqlfetchrequest_p.h"

#include <QCoreApplication>
#include <QEvent>
#include <QMutexLocker>
#include <QUrl>

#include <QtDebug>

#include <QSparqlConnection>
#include <QSparqlResult>

#include <QContactAvatar>
#include <QContactName>
#include <QContactPhoneNumber>

#include <QElapsedTimer>

SparqlFetchRequest::SparqlFetchRequest(QObject *parent)
    : QThread(parent)
    , m_state(QContactAbstractRequest::InactiveState)
    , m_threadState(QContactAbstractRequest::InactiveState)
    , m_queryData(false)
    , m_threadQueryData(false)
    , m_favoritesOnly(false)
    , m_threadFavoritesOnly(false)
    , m_sortOnFirstName(true)
    , m_threadSortOnFirstName(true)
    , m_resultsAvailable(false)
{
}

SparqlFetchRequest::~SparqlFetchRequest()
{
    {
        QMutexLocker locker(&m_mutex);
        m_state = QContactAbstractRequest::InactiveState;
        m_condition.wakeOne();
    }

    wait();
}

QContactAbstractRequest::State SparqlFetchRequest::state() const
{
    return m_state;
}

bool SparqlFetchRequest::queryData() const
{
    return m_queryData;
}

void SparqlFetchRequest::setQueryData(bool query)
{
    m_queryData = query;
}

bool SparqlFetchRequest::favoritesOnly() const
{
    return  m_favoritesOnly;
}

void SparqlFetchRequest::setFavoritesOnly(bool favorites)
{
    m_favoritesOnly = favorites;
}

bool SparqlFetchRequest::sortOnFirstName() const
{
    return m_sortOnFirstName;
}

void SparqlFetchRequest::setSortOnFirstName(bool first)
{
    m_sortOnFirstName = first;
}

QList<QContactLocalId> SparqlFetchRequest::ids() const
{
    QMutexLocker locker(&m_mutex);

    return m_contactIds;
}

QList<QContact> SparqlFetchRequest::contacts() const
{
    QMutexLocker locker(&m_mutex);

    return m_contacts;
}

void SparqlFetchRequest::start()
{
    if (m_state == QContactAbstractRequest::ActiveState)
        return;

    m_contacts.clear();
    m_contactIds.clear();

    m_threadQueryData = m_queryData;
    m_threadFavoritesOnly = m_favoritesOnly;
    m_threadSortOnFirstName = m_sortOnFirstName;

    {
        QMutexLocker locker(&m_mutex);

        m_state = QContactAbstractRequest::ActiveState;

        if (m_threadState == QContactAbstractRequest::InactiveState) {
            // The thread has is in the process of winding down.  Wait before starting it again.
             if (isRunning())
                 wait();
             m_threadState = QContactAbstractRequest::ActiveState;
             QThread::start(QThread::IdlePriority);
        }

        m_threadState = QContactAbstractRequest::ActiveState;
        m_condition.wakeOne();
    }
}

void SparqlFetchRequest::cancel()
{
    if (m_state == QContactAbstractRequest::ActiveState) {
        {
            QMutexLocker locker(&m_mutex);
            m_state = QContactAbstractRequest::CanceledState;

            m_condition.wakeOne();
        }
        emit stateChanged(m_state);
    }
}

void SparqlFetchRequest::reset()
{
    if (m_state == QContactAbstractRequest::ActiveState) {
        qWarning() << "SparqlFetchRequest: An active request cannot be reset";
        return;
    }

    m_contacts.clear();
    m_contactIds.clear();
}

bool SparqlFetchRequest::event(QEvent *event)
{
    if (event->type() == QEvent::UpdateRequest) {
        bool haveResults = false;
        QContactAbstractRequest::State previousState = m_state;

        {
            QMutexLocker locker(&m_mutex);
            haveResults = m_resultsAvailable;
            m_resultsAvailable = false;
            m_state = m_threadState;
        }

        if (haveResults)
            emit resultsAvailable();

        if (m_state != previousState)
            emit stateChanged(m_state);

        return true;
    } else {
        return QThread::event(event);
    }
}

void SparqlFetchRequest::run()
{
    const QStringList drivers = QSparqlConnection::drivers();

    QSparqlConnection connection(drivers.contains(QLatin1String("QTRACKER_DIRECT")) || drivers.isEmpty()
            ? QLatin1String("QTRACKER_DIRECT")
            : drivers.first());

    if (!connection.isValid()) {
        qWarning() << "Seaside: No valid Sparql driver, all queries will fail";
        // Allow the thread to run anyway so requests are processed and (empty)
        // result sets are returned.
    }

    QMutexLocker locker(&m_mutex);

    while (m_state != QContactAbstractRequest::InactiveState) {
        if (m_state == QContactAbstractRequest::CanceledState) {
            m_threadState = m_state;

            m_condition.wait(&m_mutex);
        } else if (m_threadState == QContactAbstractRequest::ActiveState) {
            const bool queryData = m_threadQueryData;
            const bool favoritesOnly = m_threadFavoritesOnly;
            const bool sortOnFirstName = m_threadSortOnFirstName;

            locker.unlock();
            QContactAbstractRequest::State state = executeQuery(
                        &connection, queryData, favoritesOnly, sortOnFirstName);
            locker.relock();
            m_threadState = state;
            if (!m_resultsAvailable) {
                // There's no outstanding update request.
                QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
            }
        } else { // Idle
            m_condition.wait(&m_mutex);
        }
    }
}

QContactAbstractRequest::State SparqlFetchRequest::executeQuery(
        QSparqlConnection *connection, bool queryData, bool favoritesOnly, bool orderByGivenName)
{
    const QString primary = orderByGivenName ? QLatin1String("?given") : QLatin1String("?family");
    const QString secondary = orderByGivenName ? QLatin1String("?family") : QLatin1String("?given");

    // This calculates a display name as part of the query which is then stored in the
    // customLabel property of QContactName.  If a displayName was calculated when the
    // contact was saved and stored as the nco:fullname a lot of this query could be
    // culled.  That requires co-operation between everyone who writes to the contacts
    // database, so for now I'm playing it safe.

    QString queryString = QLatin1String(
            "SELECT "
            "\n tracker:id(?x)");
    if (queryData) {
        queryString += QLatin1String(
                "\n tracker:coalesce("
                "\n  fn:string-join((%1, %2), ' '),"
                "\n  nco:imID(?imAccount),"
                "\n  nco:fullname(?organization),"
                "\n  nco:emailAddress(?emailAddress),"
                "\n  nco:fullname(?organization),"
                "\n  nco:phoneNumber(?phoneNumber),"
                "\n  '(Unnamed)')"
                "\n ?given"
                "\n ?family"
                "\n tracker:coalesce("
                "\n  nie:url(nco:photo(?x)),"
                "\n  nco:imAvatar(?imAccount))"
                "\n GROUP_CONCAT(nco:phoneNumber(?phoneNumber), ';')");
    }

    queryString += QLatin1String(
                "\nWHERE {"
                "\n ?x a nco:PersonContact .");

    if (favoritesOnly) {
        queryString += QLatin1String(
                "\n ?x nao:hasProperty ?favoriteProperty ."
                "\n ?favoriteProperty a nao:Property ."
                "\n ?favoriteProperty nao:propertyName 'Favorite' ."
                "\n ?favoriteProperty nao:hasProperty ?favorite ."
                "\n ?favorite nao:propertyValue 'true' .");
    }

    queryString += QLatin1String(
            "\n OPTIONAL { ?x nco:nameGiven ?given }"
            "\n OPTIONAL { ?x nco:nameFamily ?family }"
            "\n OPTIONAL { ?x nco:hasAffiliation ?im . ?im nco:hasIMAddress ?imAccount }"
            "\n OPTIONAL { ?x nco:hasAffiliation ?org . ?org nco:org ?organization }"
            "\n OPTIONAL { ?x nco:hasAffiliation ?email . ?email nco:hasEmailAddress ?emailAddress }"
            "\n OPTIONAL { ?x nco:hasAffiliation ?phone . ?phone nco:hasPhoneNumber ?phoneNumber  }"
            "\n FILTER(?x != <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#default-contact-me>)"
            "\n}"
            "\nGROUP BY"
            "\n ?x"
            "\nORDER BY"
            "\n ASC(tracker:coalesce("
            "\n  %1,"
            "\n  %2,"
            "\n  nco:emailAddress(?emailAddress),"
            "\n  nco:imID(?imAccount),"
            "\n  nco:fullname(?organization),"
            "\n  nco:phoneNumber(?phoneNumber),"
            "\n  '(Unnamed)'))"
            "\n ASC(%2)");

    queryString = queryString.arg(primary).arg(secondary);

    QScopedPointer<QSparqlResult> result(connection->syncExec(QSparqlQuery(queryString)));
    if (!result)
        return QContactAbstractRequest::FinishedState;

    return queryData
            ? readContacts(result.data())
            : readContactIds(result.data());
}

QContactAbstractRequest::State SparqlFetchRequest::readContacts(QSparqlResult *results)
{
    for (;;) {
        QList<QContact> contacts;
        contacts.reserve(results->size());
        for (int i = 0; i < 100 && results->next(); ++i) {
            QContact contact;
            QContactId contactId;
            contactId.setLocalId(results->value(0).value<QContactLocalId>());
            contact.setId(contactId);

            QContactName name;
            QVariant value = results->value(1);
            if (value.isValid())
                name.setCustomLabel(value.toString());
            value = results->value(2);
            if (value.isValid())
                name.setFirstName(value.toString());
            value = results->value(3);
            if (value.isValid())
                name.setLastName(value.toString());
            contact.saveDetail(&name);

            value = results->value(4);
            if (value.isValid()) {
                QContactAvatar avatar;
                avatar.setImageUrl(value.toString());
                contact.saveDetail(&avatar);
            }

            foreach (const QString &string, results->value(5).toString().split(QLatin1Char(';'))) {
                QContactPhoneNumber number;
                number.setNumber(string);
                contact.saveDetail(&number);
            }

            contacts.append(contact);
        }

        if (!contacts.isEmpty()) {
            QMutexLocker locker(&m_mutex);
            if (m_state == QContactAbstractRequest::CanceledState)
                return m_state;

            m_contacts += contacts;

            if (!m_resultsAvailable) {
                m_resultsAvailable = true;
                QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
            }
        } else {
            return QContactAbstractRequest::FinishedState;
        }
    }
}

QContactAbstractRequest::State SparqlFetchRequest::readContactIds(QSparqlResult *results)
{
    for (;;) {
        QList<QContactLocalId> contactIds;
        contactIds.reserve(results->size());
        for (int i = 0; i < 100 && results->next(); ++i)
            contactIds.append(results->value(0).value<QContactLocalId>());

        if (!contactIds.isEmpty()) {
            QMutexLocker locker(&m_mutex);
            if (m_state == QContactAbstractRequest::CanceledState)
                return m_state;

            m_contactIds += contactIds;

            if (!m_resultsAvailable) {
                m_resultsAvailable = true;
                QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
            }
        } else {
            return QContactAbstractRequest::FinishedState;
        }
    }
}
