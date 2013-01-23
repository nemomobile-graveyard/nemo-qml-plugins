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

#include <QObject>
#include <QtTest>

#include "seasidefilteredmodel.h"
#include "seasidecache.h"

Q_DECLARE_METATYPE(QModelIndex)

class tst_SeasideFilteredModel : public QObject
{
    Q_OBJECT
public:
    tst_SeasideFilteredModel();

private slots:
    void init();
    void populated();
    void filterType();
    void filterPattern();
    void rowsInserted();
    void rowsRemoved();
    void dataChanged();
    void data();
    void filterId();

private:
    SeasideCache cache;
};

tst_SeasideFilteredModel::tst_SeasideFilteredModel()
{
    qRegisterMetaType<QModelIndex>();
}


void tst_SeasideFilteredModel::init()
{
    cache.reset();
}

void tst_SeasideFilteredModel::populated()
{
    SeasideFilteredModel model;

    QCOMPARE(model.isPopulated(), false);

    QSignalSpy spy(&model, SIGNAL(populatedChanged()));

    cache.populate(SeasideFilteredModel::FilterFavorites);

    QCOMPARE(spy.count(), 0);
    model.setFilterType(SeasideFilteredModel::FilterFavorites);
    QCOMPARE(model.isPopulated(), true);
    QCOMPARE(spy.count(), 1);

    model.setFilterType(SeasideFilteredModel::FilterAll);
    QCOMPARE(model.isPopulated(), false);
    QCOMPARE(spy.count(), 2);

    cache.populate(SeasideFilteredModel::FilterAll);
    QCOMPARE(model.isPopulated(), true);
    QCOMPARE(spy.count(), 3);
}

void tst_SeasideFilteredModel::filterType()
{
    cache.insert(SeasideFilteredModel::FilterAll, 0, QVector<QContactLocalId>()
            << 0 << 1 << 2 << 3 << 4 << 5);
    cache.insert(SeasideFilteredModel::FilterFavorites, 0, QVector<QContactLocalId>()
            << 2 << 5);

    SeasideFilteredModel model;
    QSignalSpy typeSpy(&model, SIGNAL(filterTypeChanged()));
    QSignalSpy insertedSpy(&model, SIGNAL(rowsInserted(QModelIndex,int,int)));
    QSignalSpy removedSpy(&model, SIGNAL(rowsRemoved(QModelIndex,int,int)));

    //
    QCOMPARE(model.filterType(), SeasideFilteredModel::FilterNone);
    QCOMPARE(model.rowCount(), 0);

    //
    model.setFilterType(SeasideFilteredModel::FilterNone);
    QCOMPARE(model.filterType(), SeasideFilteredModel::FilterNone);
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(typeSpy.count(), 0);

    // 0 1 2 3 4 5
    model.setFilterType(SeasideFilteredModel::FilterAll);
    QCOMPARE(model.filterType(), SeasideFilteredModel::FilterAll);
    QCOMPARE(model.rowCount(), 6);
    QCOMPARE(typeSpy.count(), 1);
    QCOMPARE(insertedSpy.count(), 1);
    QCOMPARE(insertedSpy.at(0).at(1).value<int>(), 0); QCOMPARE(insertedSpy.at(0).at(2).value<int>(), 5);
    QCOMPARE(removedSpy.count(), 0);

    typeSpy.clear();
    insertedSpy.clear();

    // 2 5
    model.setFilterType(SeasideFilteredModel::FilterFavorites);
    QCOMPARE(model.filterType(), SeasideFilteredModel::FilterFavorites);
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(typeSpy.count(), 1);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 3);   // This is more noisy than it could be.
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 0); QCOMPARE(removedSpy.at(0).at(2).value<int>(), 1);
    QCOMPARE(removedSpy.at(1).at(1).value<int>(), 1); QCOMPARE(removedSpy.at(1).at(2).value<int>(), 1);
    QCOMPARE(removedSpy.at(2).at(1).value<int>(), 1); QCOMPARE(removedSpy.at(2).at(2).value<int>(), 1);

    typeSpy.clear();
    removedSpy.clear();

    // 0 1 2 3 4 5
    model.setFilterType(SeasideFilteredModel::FilterAll);
    QCOMPARE(model.filterType(), SeasideFilteredModel::FilterAll);
    QCOMPARE(model.rowCount(), 6);
    QCOMPARE(typeSpy.count(), 1);
    QCOMPARE(insertedSpy.count(), 3);   // Noise.
    QCOMPARE(insertedSpy.at(0).at(1).value<int>(), 0); QCOMPARE(insertedSpy.at(0).at(2).value<int>(), 1);
    QCOMPARE(insertedSpy.at(1).at(1).value<int>(), 3); QCOMPARE(insertedSpy.at(1).at(2).value<int>(), 3);
    QCOMPARE(insertedSpy.at(2).at(1).value<int>(), 4); QCOMPARE(insertedSpy.at(2).at(2).value<int>(), 4);
    QCOMPARE(removedSpy.count(), 0);

    typeSpy.clear();
    insertedSpy.clear();

    //
    model.setFilterType(SeasideFilteredModel::FilterNone);
    QCOMPARE(model.filterType(), SeasideFilteredModel::FilterNone);
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(typeSpy.count(), 1);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 0); QCOMPARE(removedSpy.at(0).at(2).value<int>(), 5);
}

void tst_SeasideFilteredModel::filterPattern()
{
    cache.insert(SeasideFilteredModel::FilterAll, 0, QVector<QContactLocalId>()
            << 0 << 1 << 2 << 3 << 4 << 5);
    cache.insert(SeasideFilteredModel::FilterFavorites, 0, QVector<QContactLocalId>()
            << 2 << 5);

    SeasideFilteredModel model;
    QSignalSpy patternSpy(&model, SIGNAL(filterPatternChanged()));
    QSignalSpy insertedSpy(&model, SIGNAL(rowsInserted(QModelIndex,int,int)));
    QSignalSpy removedSpy(&model, SIGNAL(rowsRemoved(QModelIndex,int,int)));

    QCOMPARE(model.filterType(), SeasideFilteredModel::FilterNone);
    QCOMPARE(model.filterPattern(), QString());
    QCOMPARE(model.rowCount(), 0);

    // 0 1 2 3 4
    model.setFilterPattern("a");
    QCOMPARE(model.filterPattern(), QString("a"));
    QCOMPARE(patternSpy.count(), 1);
    QCOMPARE(model.rowCount(), 5);
    QCOMPARE(insertedSpy.count(), 1);
    QCOMPARE(insertedSpy.at(0).at(1).value<int>(), 0); QCOMPARE(insertedSpy.at(0).at(2).value<int>(), 4);
    QCOMPARE(removedSpy.count(), 0);

    patternSpy.clear();
    insertedSpy.clear();

    // 0 1 2 4
    model.setFilterPattern("aA");
    QCOMPARE(model.filterPattern(), QString("aA"));
    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(patternSpy.count(), 1);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 3); QCOMPARE(removedSpy.at(0).at(2).value<int>(), 3);

    patternSpy.clear();
    removedSpy.clear();

    // 0 4
    model.setFilterPattern("aaronso");
    QCOMPARE(model.filterPattern(), QString("aaronso"));
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(patternSpy.count(), 1);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 1); QCOMPARE(removedSpy.at(0).at(2).value<int>(), 2);

    patternSpy.clear();
    removedSpy.clear();

    //
    model.setFilterType(SeasideFilteredModel::FilterFavorites);
    QCOMPARE(model.filterPattern(), QString("aaronso"));
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(patternSpy.count(), 0);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 0); QCOMPARE(removedSpy.at(0).at(2).value<int>(), 1);

    removedSpy.clear();

    // 2
    model.setFilterPattern("Aa");
    QCOMPARE(model.filterPattern(), QString("Aa"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(patternSpy.count(), 1);
    QCOMPARE(insertedSpy.count(), 1);
    QCOMPARE(insertedSpy.at(0).at(1).value<int>(), 0); QCOMPARE(insertedSpy.at(0).at(2).value<int>(), 0);
    QCOMPARE(removedSpy.count(), 0);

    patternSpy.clear();
    insertedSpy.clear();

    // 0 1 2 4
    model.setFilterType(SeasideFilteredModel::FilterAll);
    QCOMPARE(model.filterPattern(), QString("Aa"));
    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(patternSpy.count(), 0);
    QCOMPARE(insertedSpy.count(), 3);   // Noise.
    QCOMPARE(insertedSpy.at(0).at(1).value<int>(), 0); QCOMPARE(insertedSpy.at(0).at(2).value<int>(), 0);
    QCOMPARE(insertedSpy.at(1).at(1).value<int>(), 1); QCOMPARE(insertedSpy.at(1).at(2).value<int>(), 1);
    QCOMPARE(insertedSpy.at(2).at(1).value<int>(), 3); QCOMPARE(insertedSpy.at(2).at(2).value<int>(), 3);
    QCOMPARE(removedSpy.count(), 0);

    insertedSpy.clear();

    // 0 1 2 3 4
    model.setFilterPattern("a");
    QCOMPARE(model.filterPattern(), QString("a"));
    QCOMPARE(model.rowCount(), 5);
    QCOMPARE(patternSpy.count(), 1);
    QCOMPARE(insertedSpy.count(), 1);
    QCOMPARE(insertedSpy.at(0).at(1).value<int>(), 3); QCOMPARE(insertedSpy.at(0).at(2).value<int>(), 3);
    QCOMPARE(removedSpy.count(), 0);

    patternSpy.clear();
    insertedSpy.clear();

    // 2 3 5
    model.setFilterPattern("Jo");
    QCOMPARE(model.filterPattern(), QString("Jo"));
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(patternSpy.count(), 1);
    QCOMPARE(insertedSpy.count(), 1);
    QCOMPARE(insertedSpy.at(0).at(1).value<int>(), 2); QCOMPARE(insertedSpy.at(0).at(2).value<int>(), 2);
    QCOMPARE(removedSpy.count(), 2);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 0); QCOMPARE(removedSpy.at(0).at(2).value<int>(), 1);
    QCOMPARE(removedSpy.at(1).at(1).value<int>(), 2); QCOMPARE(removedSpy.at(1).at(2).value<int>(), 2);

    patternSpy.clear();
    insertedSpy.clear();
    removedSpy.clear();

    // 0 1 2 3 4 5
    model.setFilterPattern(QString());
    QCOMPARE(model.filterPattern(), QString());
    QCOMPARE(model.rowCount(), 6);
    QCOMPARE(patternSpy.count(), 1);
    QCOMPARE(insertedSpy.count(), 2);
    QCOMPARE(insertedSpy.at(0).at(1).value<int>(), 0); QCOMPARE(insertedSpy.at(0).at(2).value<int>(), 1);
    QCOMPARE(insertedSpy.at(1).at(1).value<int>(), 4); QCOMPARE(insertedSpy.at(1).at(2).value<int>(), 4);
    QCOMPARE(removedSpy.count(), 0);

    patternSpy.clear();
    insertedSpy.clear();

    // 2 3 4 5
    model.setFilterType(SeasideFilteredModel::FilterNone);
    model.setFilterPattern("J");
    QCOMPARE(model.rowCount(), 4);

    patternSpy.clear();
    insertedSpy.clear();
    removedSpy.clear();

    model.setFilterPattern(QString());
    QCOMPARE(model.filterPattern(), QString());
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(patternSpy.count(), 1);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 0); QCOMPARE(removedSpy.at(0).at(2).value<int>(), 3);
}

void tst_SeasideFilteredModel::rowsInserted()
{
    cache.insert(SeasideFilteredModel::FilterAll, 0, QVector<QContactLocalId>()
            << 2 << 5);

    SeasideFilteredModel model;
    model.setFilterType(SeasideFilteredModel::FilterAll);
    QSignalSpy insertedSpy(&model, SIGNAL(rowsInserted(QModelIndex,int,int)));

    // 2 5
    QCOMPARE(model.rowCount(), 2);

    // 0 1 2 5
    cache.insert(SeasideFilteredModel::FilterAll, 0, QVector<QContactLocalId>()
            << 0 << 1);
    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(insertedSpy.count(), 1);
    QCOMPARE(insertedSpy.at(0).at(1).value<int>(), 0); QCOMPARE(insertedSpy.at(0).at(2).value<int>(), 1);

    insertedSpy.clear();

    // 1
    model.setFilterPattern("Ar");
    QCOMPARE(model.rowCount(), 1);

    // 1 3
    cache.insert(SeasideFilteredModel::FilterAll, 3, QVector<QContactLocalId>()
            << 3 << 4);
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(insertedSpy.count(), 1);
    QCOMPARE(insertedSpy.at(0).at(1).value<int>(), 1); QCOMPARE(insertedSpy.at(0).at(2).value<int>(), 1);
}

void tst_SeasideFilteredModel::rowsRemoved()
{
    cache.insert(SeasideFilteredModel::FilterAll, 0, QVector<QContactLocalId>()
            << 0 << 1 << 2 << 3 << 4 << 5);

    SeasideFilteredModel model;
    model.setFilterType(SeasideFilteredModel::FilterAll);
    QSignalSpy removedSpy(&model, SIGNAL(rowsRemoved(QModelIndex,int,int)));

    // 0 1 2 3 4 5
    QCOMPARE(model.rowCount(), 6);

    // 2 3 4 5
    cache.remove(SeasideFilteredModel::FilterAll, 0, 2);
    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 0); QCOMPARE(removedSpy.at(0).at(2).value<int>(), 1);

    //  2 3 5
    model.setFilterPattern("Jo");
    QCOMPARE(model.rowCount(), 3);

    removedSpy.clear();

    // 2 5
    cache.remove(SeasideFilteredModel::FilterAll, 1, 2);
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 1); QCOMPARE(removedSpy.at(0).at(2).value<int>(), 1);
}

void tst_SeasideFilteredModel::dataChanged()
{
    cache.insert(SeasideFilteredModel::FilterAll, 0, QVector<QContactLocalId>()
            << 0 << 1 << 2 << 3 << 4 << 5);

    SeasideFilteredModel model;
    model.setFilterType(SeasideFilteredModel::FilterAll);

    // 0 1 2 3 4 5
    QCOMPARE(model.rowCount(), 6);

    QSignalSpy insertedSpy(&model, SIGNAL(rowsInserted(QModelIndex,int,int)));
    QSignalSpy removedSpy(&model, SIGNAL(rowsRemoved(QModelIndex,int,int)));
    QSignalSpy changedSpy(&model, SIGNAL(dataChanged(QModelIndex,QModelIndex)));

    // 0 1 2 3 4 5
    cache.setDisplayName(SeasideFilteredModel::FilterAll, 2, "Doug");
    QCOMPARE(model.rowCount(), 6);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 0);
    QCOMPARE(changedSpy.count(), 1);
    QCOMPARE(changedSpy.at(0).at(0).value<QModelIndex>(), model.index(QModelIndex(), 2, 0));
    QCOMPARE(changedSpy.at(0).at(1).value<QModelIndex>(), model.index(QModelIndex(), 2, 0));

    // 0 1 3 4
    model.setFilterPattern("A");
    QCOMPARE(model.rowCount(), 4);

    removedSpy.clear();
    changedSpy.clear();

    cache.setDisplayName(SeasideFilteredModel::FilterAll, 2, "Aaron Johns");
    QCOMPARE(model.rowCount(), 5);
    QCOMPARE(insertedSpy.count(), 1);
    QCOMPARE(insertedSpy.at(0).at(1).value<int>(), 2); QCOMPARE(insertedSpy.at(0).at(2).value<int>(), 2);
    QCOMPARE(removedSpy.count(), 0);
    QCOMPARE(changedSpy.count(), 0);

    insertedSpy.clear();

    // 0 1 2 3 4 5
    cache.setDisplayName(SeasideFilteredModel::FilterAll, 2, "Doug");
    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 2); QCOMPARE(removedSpy.at(0).at(2).value<int>(), 2);
    QCOMPARE(changedSpy.count(), 0);
}

void tst_SeasideFilteredModel::data()
{
    cache.insert(SeasideFilteredModel::FilterAll, 0, QVector<QContactLocalId>()
            << 0 << 1 << 2 << 3 << 4 << 5);
    cache.insert(SeasideFilteredModel::FilterFavorites, 0, QVector<QContactLocalId>()
            << 2 << 5);

    QModelIndex index;

    SeasideFilteredModel model;
    model.setFilterType(SeasideFilteredModel::FilterAll);

    QCOMPARE(model.rowCount(), 6);

    index = model.index(QModelIndex(), 0, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), QString("Aaron Aaronson"));
    QCOMPARE(index.data(SeasideFilteredModel::FirstNameRole).toString(), QString("Aaron"));
    QCOMPARE(index.data(SeasideFilteredModel::LastNameRole).toString(), QString("Aaronson"));
    QCOMPARE(index.data(SeasideFilteredModel::SectionBucketRole).toString(), QString("A"));
    QCOMPARE(index.data(SeasideFilteredModel::AvatarRole).toUrl(), QUrl(QLatin1String("image://theme/icon-m-telephony-contact-avatar")));

    index = model.index(QModelIndex(), 5, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), QString("Joe Johns"));
    QCOMPARE(index.data(SeasideFilteredModel::FirstNameRole).toString(), QString("Joe"));
    QCOMPARE(index.data(SeasideFilteredModel::LastNameRole).toString(), QString("Johns"));
    QCOMPARE(index.data(SeasideFilteredModel::SectionBucketRole).toString(), QString("J"));
    QCOMPARE(index.data(SeasideFilteredModel::AvatarRole).toUrl(), QUrl(QLatin1String("file:///cache/joe.jpg")));

    model.setFilterType(SeasideFilteredModel::FilterFavorites);

    index = model.index(QModelIndex(), 0, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), QString("Aaron Johns"));
    QCOMPARE(index.data(SeasideFilteredModel::FirstNameRole).toString(), QString("Aaron"));
    QCOMPARE(index.data(SeasideFilteredModel::LastNameRole).toString(), QString("Johns"));
    QCOMPARE(index.data(SeasideFilteredModel::SectionBucketRole).toString(), QString("A"));
    QCOMPARE(index.data(SeasideFilteredModel::AvatarRole).toUrl(), QUrl(QLatin1String("image://theme/icon-m-telephony-contact-avatar")));

    index = model.index(QModelIndex(), 1, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), QString("Joe Johns"));
    QCOMPARE(index.data(SeasideFilteredModel::FirstNameRole).toString(), QString("Joe"));
    QCOMPARE(index.data(SeasideFilteredModel::LastNameRole).toString(), QString("Johns"));
    QCOMPARE(index.data(SeasideFilteredModel::SectionBucketRole).toString(), QString("J"));
    QCOMPARE(index.data(SeasideFilteredModel::AvatarRole).toUrl(), QUrl(QLatin1String("file:///cache/joe.jpg")));
}

void tst_SeasideFilteredModel::filterId()
{
    SeasideFilteredModel model;
    // 6: Robin Burchell

    // first name only
    model.setFilterPattern("R");            QVERIFY(model.filterId(6));
    model.setFilterPattern("Ro");           QVERIFY(model.filterId(6));
    model.setFilterPattern("Rob");          QVERIFY(model.filterId(6));
    model.setFilterPattern("Robi");         QVERIFY(model.filterId(6));
    model.setFilterPattern("Robin");        QVERIFY(model.filterId(6));

    // match using last name
    model.setFilterPattern("B");            QVERIFY(model.filterId(6));
    model.setFilterPattern("Bu");           QVERIFY(model.filterId(6));
    model.setFilterPattern("Bur");          QVERIFY(model.filterId(6));
    model.setFilterPattern("Burc");         QVERIFY(model.filterId(6));
    model.setFilterPattern("Burch");        QVERIFY(model.filterId(6));
    model.setFilterPattern("Burche");       QVERIFY(model.filterId(6));
    model.setFilterPattern("Burchel");      QVERIFY(model.filterId(6));
    model.setFilterPattern("Burchell");     QVERIFY(model.filterId(6));


    // first name plus last name
    model.setFilterPattern("Robin ");           QVERIFY(model.filterId(6));
    model.setFilterPattern("Robin B");          QVERIFY(model.filterId(6));
    model.setFilterPattern("Robin Bu");         QVERIFY(model.filterId(6));
    model.setFilterPattern("Robin Bur");        QVERIFY(model.filterId(6));
    model.setFilterPattern("Robin Burc");       QVERIFY(model.filterId(6));
    model.setFilterPattern("Robin Burch");      QVERIFY(model.filterId(6));
    model.setFilterPattern("Robin Burche");     QVERIFY(model.filterId(6));
    model.setFilterPattern("Robin Burchel");    QVERIFY(model.filterId(6));
    model.setFilterPattern("Robin Burchell");   QVERIFY(model.filterId(6));

    // match using funky spacing
    model.setFilterPattern("R B");  QVERIFY(model.filterId(6));
    model.setFilterPattern("R Bu"); QVERIFY(model.filterId(6));

    // test some that most definitely shouldn't match
    model.setFilterPattern("Robert");           QVERIFY(!model.filterId(6));
    model.setFilterPattern("Robin Brooks");     QVERIFY(!model.filterId(6));
    model.setFilterPattern("John Burchell");    QVERIFY(!model.filterId(6));
    model.setFilterPattern("Brooks");           QVERIFY(!model.filterId(6));
}

#include "tst_seasidefilteredmodel.moc"
QTEST_APPLESS_MAIN(tst_SeasideFilteredModel)
