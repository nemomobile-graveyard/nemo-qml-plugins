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

#ifndef SEASIDEFILTEREDMODEL_H
#define SEASIDEFILTEREDMODEL_H


#include <QAbstractListModel>
#include <QStringList>
#include <QVector>

#include <QContactId>

class SeasidePerson;

QTM_USE_NAMESPACE

class SeasideFilteredModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool populated READ isPopulated NOTIFY populatedChanged)
    Q_PROPERTY(FilterType filterType READ filterType WRITE setFilterType NOTIFY filterTypeChanged)
    Q_PROPERTY(DisplayLabelOrder displayLabelOrder READ displayLabelOrder WRITE setDisplayLabelOrder NOTIFY displayLabelOrderChanged)
    Q_PROPERTY(QString filterPattern READ filterPattern WRITE setFilterPattern NOTIFY filterPatternChanged)
    Q_PROPERTY(bool searchByFirstNameCharacter READ searchByFirstNameCharacter WRITE setSearchByFirstNameCharacter NOTIFY searchByFirstNameCharacterChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_ENUMS(FilterType DisplayLabelOrder)
public:
    enum FilterType {
        FilterNone,
        FilterAll,
        FilterFavorites,
        FilterOnline,
        FilterTypesCount
    };

    enum DisplayLabelOrder {
        FirstNameFirst,
        LastNameFirst
    };

    enum PeopleRoles {
        FirstNameRole = Qt::UserRole,
        LastNameRole,
        AvatarRole,
        SectionBucketRole,
        PersonRole
    };

    SeasideFilteredModel(QObject *parent = 0);
    ~SeasideFilteredModel();

    bool isPopulated() const;

    FilterType filterType() const;
    void setFilterType(FilterType type);

    QString filterPattern() const;
    void setFilterPattern(const QString &pattern);

    bool searchByFirstNameCharacter() const;
    void setSearchByFirstNameCharacter(bool searchByFirstNameCharacter);

    DisplayLabelOrder displayLabelOrder() const;
    void setDisplayLabelOrder(DisplayLabelOrder order);

    Q_INVOKABLE QVariantMap get(int row) const;

    Q_INVOKABLE bool savePerson(SeasidePerson *person);
    Q_INVOKABLE SeasidePerson *personByRow(int row) const;
    Q_INVOKABLE SeasidePerson *personById(int id) const;
    Q_INVOKABLE SeasidePerson *personByPhoneNumber(const QString &msisdn) const;
    Q_INVOKABLE SeasidePerson *selfPerson() const;
    Q_INVOKABLE void removePerson(SeasidePerson *person);

    Q_INVOKABLE int importContacts(const QString &path);
    Q_INVOKABLE QString exportContacts();

    QModelIndex index(const QModelIndex &parent, int row, int column) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;

    // SeasideProxyModel compatibilty naming.
    Q_INVOKABLE void setFilter(FilterType type) { setFilterType(type); }
    Q_INVOKABLE void search(const QString &pattern) { setFilterPattern(pattern); }

    // For SeasideCache.
    void sourceAboutToRemoveItems(int begin, int end);
    void sourceItemsRemoved();

    void sourceAboutToInsertItems(int begin, int end);
    void sourceItemsInserted(int begin, int end);

    void sourceDataChanged(int begin, int end);

    void makePopulated();
    void updateDisplayLabelOrder();

    // For synchronizeLists()
    bool filterId(QContactLocalId contactId) const;
    void insertRange(int index, int count, const QVector<QContactLocalId> &source, int sourceIndex);
    void removeRange(int index, int count);

signals:
    void populatedChanged();
    void filterTypeChanged();
    void filterPatternChanged();
    void searchByFirstNameCharacterChanged();
    void displayLabelOrderChanged();
    void countChanged();

private:
    void populateIndex();
    void refineIndex();
    void updateIndex();
    void updateContactData(QContactLocalId contactId, SeasideFilteredModel::FilterType filter);

    QVector<QContactLocalId> m_filteredContactIds;
    const QVector<QContactLocalId> *m_contactIds;
    const QVector<QContactLocalId> *m_referenceContactIds;
    QStringList m_filterParts;
    QString m_filterPattern;
    int m_filterIndex;
    int m_referenceIndex;
    FilterType m_filterType;
    bool m_searchByFirstNameCharacter;
};

#endif
