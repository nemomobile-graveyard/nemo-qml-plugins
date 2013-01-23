/*
 * Copyright 2011 Intel Corporation.
 * Copyright 2011 Robin Burchell
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 	
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <QDebug>

#include <QStringList>

#include "localeutils_p.h"
#include "seasideperson.h"
#include "seasidepeoplemodel.h"
#include "seasideproxymodel.h"

class SeasideProxyModelPriv
{
public:
    SeasideProxyModelPriv()
        : componentComplete(false)
    {
    }

    SeasideProxyModel::FilterType filterType;
    LocaleUtils *localeHelper;
    QString searchPattern;
    bool componentComplete;
};

SeasideProxyModel::SeasideProxyModel(QObject *parent)
{
    Q_UNUSED(parent);
    priv = new SeasideProxyModelPriv;
    priv->filterType = FilterAll;
    mDisplayLabelOrder = FirstNameFirst;
    priv->localeHelper = LocaleUtils::self();
    setDynamicSortFilter(true);
    setFilterKeyColumn(-1);

    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)),
            SIGNAL(countChanged()));

    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            SIGNAL(countChanged()));

    connect(this, SIGNAL(layoutChanged()),
            SIGNAL(countChanged()));       

}

SeasideProxyModel::~SeasideProxyModel()
{
    delete priv;
}

void SeasideProxyModel::classBegin()
{
    priv->componentComplete = false;
}

void SeasideProxyModel::componentComplete()
{
    priv->componentComplete = true;

    SeasidePeopleModel *sourceModel = SeasidePeopleModel::instance();
    setSourceModel(sourceModel);
    sort(0, Qt::AscendingOrder);

    connect(sourceModel, SIGNAL(populatedChanged()),
            this, SIGNAL(populatedChanged()));

    connect(this, SIGNAL(displayLabelOrderChanged(SeasideProxyModel::DisplayLabelOrder)),
            sourceModel, SIGNAL(displayLabelOrderChanged(SeasideProxyModel::DisplayLabelOrder)));

    if (mDisplayLabelOrder != FirstNameFirst) {
        emit displayLabelOrderChanged(mDisplayLabelOrder);
    }
}

bool SeasideProxyModel::populated() const
{
    SeasidePeopleModel *model = static_cast<SeasidePeopleModel *>(sourceModel());
    if (model)
        return model->populated();
    return false;
}

SeasideProxyModel::FilterType SeasideProxyModel::filterType() const
{
    return priv->filterType;
}

void SeasideProxyModel::setFilter(FilterType filter)
{
    if (filter != priv->filterType) {
        priv->filterType = filter;
        if (filter == FilterNone)
            priv->searchPattern = QString();
        if (priv->componentComplete)
            invalidateFilter();
        emit filterTypeChanged();
        if (filter == FilterNone)
            emit filterPatternChanged();
    }
}

QString SeasideProxyModel::filterPattern() const
{
    return priv->searchPattern;
}

void SeasideProxyModel::setFilterPattern(const QString &pattern)
{
    if (pattern != priv->searchPattern) {
        priv->searchPattern = pattern;
        if (priv->componentComplete)
            invalidateFilter();
        emit filterPatternChanged();
    }
}

SeasideProxyModel::DisplayLabelOrder SeasideProxyModel::displayLabelOrder()
{
    return mDisplayLabelOrder;
}

void SeasideProxyModel::setDisplayLabelOrder(DisplayLabelOrder order)
{
    if (order != mDisplayLabelOrder) {
        mDisplayLabelOrder = order;
        invalidate();
        emit displayLabelOrderChanged(order);
    }
}

bool SeasideProxyModel::personMatchesFilter(SeasidePerson *person, const QString &filter)
{
    // split the display label and filter into words.
    //
    // TODO: i18n will require different splitting for thai and possibly
    // other locales, see MBreakIterator
    QStringList labelList = person->displayLabel().split(" ");
    QStringList filterList = filter.split(" ");

    // search forwards over the label components for each filter word, making
    // sure to find all filter words before considering it a match.
    //
    // TODO: for performance, we may want to consider keeping a cached
    // version of the parts comprising a display label -
    // QStringList SeasidePerson::displayLabelParts?
    int j = 0;
    for (int i = 0; i < filterList.size(); i++) {
        bool found = false;
        for (; j < labelList.size(); j++) {
            // TODO: for good i18n, we need to search insensitively taking
            // diacritics into account, QString's functions alone aren't good
            // enough
            if (labelList.at(j).startsWith(filterList.at(i), Qt::CaseInsensitive)) {
                found = true;
                j++;
                break;
            }
        }

        // if the current filter word wasn't found in the search
        // string, then it wasn't a match. we require all words
        // to match.
        if (!found)
            return false;
    }

    return true;
}

bool SeasideProxyModel::filterAcceptsRow(int source_row,
                                  const QModelIndex& source_parent) const
{
    // TODO: add communication history
    //if (!QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent))
    //    return false;

    // TODO: it may be better to filter this in SeasidePeopleModel, instead
    // of constantly doing it on refilter.
    SeasidePeopleModel *model = static_cast<SeasidePeopleModel *>(sourceModel());
    SeasidePerson *person = model->personByRow(source_row);
    if (person->id() == model->manager()->selfContactId())
        return false;

    if (!priv->searchPattern.isEmpty()) {
        if (!personMatchesFilter(person, priv->searchPattern))
            return false;
    }

    switch (priv->filterType) {
        case FilterNone:
            return false;
        case FilterAll:
            return true;
        case FilterFavorites:
            return person->favorite();
    }

    return false;
}

bool SeasideProxyModel::lessThan(const QModelIndex& left,
                                 const QModelIndex& right) const
{
    SeasidePeopleModel *model = static_cast<SeasidePeopleModel *>(sourceModel());

    SeasidePerson *leftPerson = model->personByRow(left.row());
    SeasidePerson *rightPerson = model->personByRow(right.row());

    if (!leftPerson)
        return false;
    else if (!rightPerson)
        return true;

    return priv->localeHelper->isLessThan(leftPerson->displayLabel(),
                                          rightPerson->displayLabel());
}

QVariantMap SeasideProxyModel::get(int row) const
{
    // needed for SectionScroller.
    QVariantMap m;
    SeasidePerson *p = personByRow(row);
    m["sectionBucket"] = p->sectionBucket();
    return m;
}

bool SeasideProxyModel::savePerson(SeasidePerson *person)
{
    SeasidePeopleModel *model = static_cast<SeasidePeopleModel *>(sourceModel());
    return model->savePerson(person);
}

SeasidePerson* SeasideProxyModel::personByRow(int row) const {
    SeasidePeopleModel *model = static_cast<SeasidePeopleModel *>(sourceModel());
    int sourceRow = mapToSource(index(row, 0)).row();
    return model->personByRow(sourceRow);
}

SeasidePerson * SeasideProxyModel::personById(int id) const
{
    SeasidePeopleModel *model = static_cast<SeasidePeopleModel *>(sourceModel());
    return model->personById(id);
}
SeasidePerson * SeasideProxyModel::personByPhoneNumber(const QString &msisdn) const
{
    SeasidePeopleModel *model = static_cast<SeasidePeopleModel*>(sourceModel());
    return model->personByPhoneNumber(msisdn);
}
void SeasideProxyModel::removePerson(SeasidePerson *person)
{
    SeasidePeopleModel *model = static_cast<SeasidePeopleModel *>(sourceModel());
    model->removePerson(person);
}

int SeasideProxyModel::importContacts(const QString &path)
{
    SeasidePeopleModel *model = static_cast<SeasidePeopleModel *>(sourceModel());
    return model->importContacts(path);
}

QString SeasideProxyModel::exportContacts()
{
    SeasidePeopleModel *model = static_cast<SeasidePeopleModel *>(sourceModel());
    return model->exportContacts();
}

int SeasideProxyModel::count() const
{
    return rowCount(QModelIndex());
}
