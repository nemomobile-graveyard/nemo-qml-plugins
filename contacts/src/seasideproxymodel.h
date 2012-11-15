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
#include "seasidepeoplemodel.h"

class SeasideProxyModelPriv;

class SeasideProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_ENUMS(FilterType)

public:
    SeasideProxyModel(QObject *parent = 0);
    virtual ~SeasideProxyModel();

    enum FilterType {
        FilterAll,
        FilterFavorites,
    };

    enum StringType {
        Primary,
        Secondary,
    };

    Q_INVOKABLE virtual void setFilter(FilterType filter);
    Q_INVOKABLE virtual void search(const QString &pattern);

    // for SectionScroller support
    Q_INVOKABLE QVariantMap get(int row) const;

    // API
    Q_INVOKABLE bool savePerson(SeasidePerson *person)
    {
        SeasidePeopleModel *model = static_cast<SeasidePeopleModel *>(sourceModel());
        return model->savePerson(person);
    }
    Q_INVOKABLE SeasidePerson *personByRow(int row) const
    {
        SeasidePeopleModel *model = static_cast<SeasidePeopleModel *>(sourceModel());
        int sourceRow = mapToSource(index(row, 0)).row();
        return model->personByRow(sourceRow);
    }
    Q_INVOKABLE SeasidePerson *personById(int id) const
    {
        SeasidePeopleModel *model = static_cast<SeasidePeopleModel *>(sourceModel());
        return model->personById(id);
    }
    Q_INVOKABLE SeasidePerson *personByPhoneNumber(const QString &msisdn) const
    {
        SeasidePeopleModel *model = static_cast<SeasidePeopleModel*>(sourceModel());
        return model->personByPhoneNumber(msisdn);
    }
    Q_INVOKABLE void removePerson(SeasidePerson *person)
    {
        SeasidePeopleModel *model = static_cast<SeasidePeopleModel *>(sourceModel());
        model->removePerson(person);
    }
    Q_INVOKABLE int importContacts(const QString &path)
    {
        SeasidePeopleModel *model = static_cast<SeasidePeopleModel *>(sourceModel());
        return model->importContacts(path);
    }
    Q_INVOKABLE QString exportContacts()
    {
        SeasidePeopleModel *model = static_cast<SeasidePeopleModel *>(sourceModel());
        return model->exportContacts();
    }

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    int count() const;

signals:
    void countChanged();

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;
    virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const;

private:
    static bool personMatchesFilter(SeasidePerson *person, const QString &filter);

    friend class tst_SeasideProxyModel;
    SeasideProxyModelPriv *priv;
    Q_DISABLE_COPY(SeasideProxyModel)
};

#endif // SEASIDEPROXYMODEL_H
