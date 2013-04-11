/*
 * Copyright (C) 2012 Robin Burchell <robin+nemo@viroteck.net>
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

#ifndef DIRMODEL_H
#define DIRMODEL_H

#include <QAbstractListModel>
#include <QFileInfo>
#include <QVector>
#include <QStringList>

#include "iorequest.h"

class DirModel : public QAbstractListModel
{
    Q_OBJECT

    enum Roles {
        FileNameRole = Qt::UserRole,
        CreationDateRole,
        ModifiedDateRole,
        FileSizeRole,
        IconSourceRole,
        FilePathRole,
        IsDirRole,
        IsFileRole,
        IsReadableRole,
        IsWritableRole,
        IsExecutableRole
    };

public:
    DirModel(QObject *parent = 0);

    int rowCount(const QModelIndex &index) const
    {
        if (index.parent() != QModelIndex())
            return 0;

        return mDirectoryContents.count();
    }

    // TODO: this won't be safe if the model can change under the holder of the row
    Q_INVOKABLE QVariant data(int row, const QByteArray &stringRole) const;

    QVariant data(const QModelIndex &index, int role) const;

    Q_INVOKABLE void refresh()
    {
        // just some syntactical sugar really
        setPath(path());
    }

    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged);
    inline QString path() const { return mCurrentDir; }

    Q_PROPERTY(QString parentPath READ parentPath NOTIFY pathChanged);
    QString parentPath() const;

    Q_INVOKABLE QString homePath() const;

    Q_PROPERTY(bool awaitingResults READ awaitingResults NOTIFY awaitingResultsChanged);
    bool awaitingResults() const;

    void setPath(const QString &pathName);

    Q_INVOKABLE void rm(const QStringList &paths);

    Q_INVOKABLE bool rename(int row, const QString &newName);

    Q_INVOKABLE void mkdir(const QString &newdir);

    Q_PROPERTY(bool showDirectories READ showDirectories WRITE setShowDirectories NOTIFY showDirectoriesChanged)
    bool showDirectories() const;
    void setShowDirectories(bool showDirectories);

    Q_PROPERTY(QStringList nameFilters READ nameFilters WRITE setNameFilters NOTIFY nameFiltersChanged)
    QStringList nameFilters() const;
    void setNameFilters(const QStringList &nameFilters);

public slots:
    void onItemsAdded(const QVector<QFileInfo> &newFiles);

signals:
    void awaitingResultsChanged();
    void nameFiltersChanged();
    void showDirectoriesChanged();
    void pathChanged();
    void error(const QString &errorTitle, const QString &errorMessage);

private:
    QHash<int, QByteArray> buildRoleNames() const;

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    // In Qt5, the roleNames() is virtual and will work just fine. On qt4 setRoleNames must be used with buildRoleNames.
    QHash<int, QByteArray> roleNames() const;
#endif

    QStringList mNameFilters;
    bool mShowDirectories;
    bool mAwaitingResults;
    QString mCurrentDir;
    QVector<QFileInfo> mDirectoryContents;
};


#endif // DIRMODEL_H
