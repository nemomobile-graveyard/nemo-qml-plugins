/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 	
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SEASIDEPROXYMODEL_H
#define SEASIDEPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QDeclarativeParserStatus>

class SeasidePerson;
class SeasideProxyModelPriv;
class SeasidePeopleModel;
class SeasideProxyModel : public QSortFilterProxyModel, public QDeclarativeParserStatus
{
    Q_OBJECT
    Q_PROPERTY(bool populated READ populated NOTIFY populatedChanged)
    Q_PROPERTY(FilterType filterType READ filterType WRITE setFilter NOTIFY filterTypeChanged)
    Q_PROPERTY(QString filterPattern READ filterPattern WRITE setFilterPattern NOTIFY filterPatternChanged)
    Q_PROPERTY(DisplayLabelOrder displayLabelOrder READ displayLabelOrder WRITE setDisplayLabelOrder NOTIFY displayLabelOrderChanged(SeasideProxyModel::DisplayLabelOrder))

    Q_ENUMS(FilterType)
    Q_ENUMS(DisplayLabelOrder)

public:
    SeasideProxyModel(QObject *parent = 0);
    virtual ~SeasideProxyModel();

    enum FilterType {
        FilterNone,
        FilterAll,
        FilterFavorites
    };

    enum DisplayLabelOrder {
        FirstNameFirst,
        LastNameFirst
    };

    enum StringType {
        Primary,
        Secondary
    };

    void classBegin();
    void componentComplete();

    bool populated() const;

    FilterType filterType() const;
    QString filterPattern() const;
    void setDisplayLabelOrder(DisplayLabelOrder displayLabelOrder);

    DisplayLabelOrder displayLabelOrder();

    Q_INVOKABLE virtual void setFilter(FilterType filter);
    Q_INVOKABLE void setFilterPattern(const QString &pattern);
    Q_INVOKABLE void search(const QString &pattern) { setFilterPattern(pattern); }

    // for SectionScroller support
    Q_INVOKABLE QVariantMap get(int row) const;

    // API
    Q_INVOKABLE bool savePerson(SeasidePerson *person);
    Q_INVOKABLE SeasidePerson *personByRow(int row) const;
    Q_INVOKABLE SeasidePerson *personById(int id) const;
    Q_INVOKABLE SeasidePerson *personByPhoneNumber(const QString &msisdn) const;
    Q_INVOKABLE void removePerson(SeasidePerson *person);
    Q_INVOKABLE int importContacts(const QString &path);
    Q_INVOKABLE QString exportContacts();

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    int count() const;

signals:
    void populatedChanged();
    void countChanged();
    void filterTypeChanged();
    void filterPatternChanged();
    void displayLabelOrderChanged(SeasideProxyModel::DisplayLabelOrder);

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;
    virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const;

private:
    static bool personMatchesFilter(SeasidePerson *person, const QString &filter);

    friend class tst_SeasideProxyModel;
    SeasideProxyModelPriv *priv;
    SeasideProxyModel::DisplayLabelOrder mDisplayLabelOrder;
    Q_DISABLE_COPY(SeasideProxyModel)
};

#endif // SEASIDEPROXYMODEL_H
