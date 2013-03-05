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

#include <QDirIterator>
#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <QUrl>

#include <errno.h>
#include <string.h>

#include "dirmodel.h"
#include "ioworkerthread.h"

Q_GLOBAL_STATIC(IOWorkerThread, ioWorkerThread);

class DirListWorker : public IORequest
{
    Q_OBJECT
public:
    DirListWorker(const QString &pathName)
        : mPathName(pathName)
    { }

    void run()
    {
        qDebug() << Q_FUNC_INFO << "Running on: " << QThread::currentThreadId();

        QDir tmpDir = QDir(mPathName);
        QDirIterator it(tmpDir);
        QVector<QFileInfo> directoryContents;

        while (it.hasNext()) {
            it.next();

            // skip hidden files
            if (it.fileName()[0] == QLatin1Char('.'))
                continue;

            directoryContents.append(it.fileInfo());
            if (directoryContents.count() >= 50) {
                emit itemsAdded(directoryContents);

                // clear() would force a deallocation, micro-optimisation
                directoryContents.erase(directoryContents.begin(), directoryContents.end());
            }
        }

        // last batch
        emit itemsAdded(directoryContents);

        //std::sort(directoryContents.begin(), directoryContents.end(), DirModel::fileCompare);
    }

signals:
    void itemsAdded(const QVector<QFileInfo> &files);

private:
    QString mPathName;
};

DirModel::DirModel(QObject *parent)
    : QAbstractListModel(parent)
    , mAwaitingResults(false)
    , mShowDirectories(true)
{
    mNameFilters = QStringList() << "*";

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    // There's no setRoleNames in Qt5.
    setRoleNames(buildRoleNames());
#endif

    // populate reverse mapping
    const QHash<int, QByteArray> &roles = roleNames();
    QHash<int, QByteArray>::ConstIterator it = roles.constBegin();
    for (;it != roles.constEnd(); ++it)
        mRoleMapping.insert(it.value(), it.key());

    // make sure we cover all roles
//    Q_ASSERT(roles.count() == IsFileRole - FileNameRole);
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
// roleNames has changed between Qt4 and Qt5. In Qt5 it is a virtual
// function and setRoleNames should not be used.
QHash<int, QByteArray> DirModel::roleNames() const
{
    static QHash<int, QByteArray> roles;
    if (roles.isEmpty()) {
        roles = buildRoleNames();
    }

    return roles;
}
#endif

QHash<int, QByteArray> DirModel::buildRoleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(FileNameRole, QByteArray("fileName"));
    roles.insert(CreationDateRole, QByteArray("creationDate"));
    roles.insert(ModifiedDateRole, QByteArray("modifiedDate"));
    roles.insert(FileSizeRole, QByteArray("fileSize"));
    roles.insert(IconSourceRole, QByteArray("iconSource"));
    roles.insert(FilePathRole, QByteArray("filePath"));
    roles.insert(IsDirRole, QByteArray("isDir"));
    roles.insert(IsFileRole, QByteArray("isFile"));
    roles.insert(IsReadableRole, QByteArray("isReadable"));
    roles.insert(IsWritableRole, QByteArray("isWritable"));
    roles.insert(IsExecutableRole, QByteArray("isExecutable"));

    return roles;
}

QVariant DirModel::data(int row, const QByteArray &stringRole) const
{
    QHash<QByteArray, int>::ConstIterator it = mRoleMapping.constFind(stringRole);

    if (it == mRoleMapping.constEnd())
        return QVariant();

    return data(index(row, 0), *it);
}

QVariant DirModel::data(const QModelIndex &index, int role) const
{
    if (role < FileNameRole || role > IsExecutableRole) {
        qWarning() << Q_FUNC_INFO << "Got an out of range role: " << role;
        return QVariant();
    }

    if (index.row() < 0 || index.row() >= mDirectoryContents.count()) {
        qWarning() << "Attempted to access out of range row: " << index.row();
        return QVariant();
    }

    if (index.column() != 0)
        return QVariant();

    const QFileInfo &fi = mDirectoryContents.at(index.row());

    switch (role) {
        case FileNameRole:
            return fi.fileName();
        case CreationDateRole:
            return fi.created();
        case ModifiedDateRole:
            return fi.lastModified();
        case FileSizeRole: {
            qint64 kb = fi.size() / 1024;
            if (kb < 1)
                return QString::number(fi.size()) + " bytes";
            else if (kb < 1024)
                return QString::number(kb) + " kb";

            kb /= 1024;
            return QString::number(kb) + "mb";
        }
        case IconSourceRole: {
            const QString &fileName = fi.fileName();

            if (fi.isDir())
                return "image://theme/icon-m-common-directory";

            if (fileName.endsWith(".jpg", Qt::CaseInsensitive) ||
                fileName.endsWith(".png", Qt::CaseInsensitive)) {
                return "image://nemoThumbnail/" + fi.filePath();
            }

            return "image://theme/icon-m-content-document";
        }
        case FilePathRole:
            return fi.filePath();
        case IsDirRole:
            return fi.isDir();
        case IsFileRole:
            return !fi.isDir();
        case IsReadableRole:
            return fi.isReadable();
        case IsWritableRole:
            return fi.isWritable();
        case IsExecutableRole:
            return fi.isExecutable();
        default:
            // this should not happen, ever
            Q_ASSERT(false);
            qWarning() << Q_FUNC_INFO << "Got an unknown role: " << role;
            return QVariant();
    }
}

void DirModel::setPath(const QString &pathName)
{
    if (pathName.isEmpty())
        return;

    if (mAwaitingResults) {
        // TODO: handle the case where pathName != our current path, cancel old
        // request, start a new one
        qDebug() << Q_FUNC_INFO << "Ignoring path change request, request already running";
        return;
    }

    mAwaitingResults = true;
    emit awaitingResultsChanged();
    qDebug() << Q_FUNC_INFO << "Changing to " << pathName << " on " << QThread::currentThreadId();

    beginResetModel();
    mDirectoryContents.clear();
    endResetModel();

    // TODO: we need to set a spinner active before we start getting results from DirListWorker
    DirListWorker *dlw = new DirListWorker(pathName);
    connect(dlw, SIGNAL(itemsAdded(QVector<QFileInfo>)), SLOT(onItemsAdded(QVector<QFileInfo>)));
    ioWorkerThread()->addRequest(dlw);

    mCurrentDir = pathName;
    emit pathChanged();
}

static bool fileCompare(const QFileInfo &a, const QFileInfo &b)
{
    if (a.isDir() && !b.isDir())
        return true;

    if (b.isDir() && !a.isDir())
        return false;

    return QString::localeAwareCompare(a.fileName(), b.fileName()) < 0;
}

void DirModel::onItemsAdded(const QVector<QFileInfo> &newFiles)
{
    qDebug() << Q_FUNC_INFO << "Got new files: " << newFiles.count();

    if (mAwaitingResults) {
        qDebug() << Q_FUNC_INFO << "No longer awaiting results";
        mAwaitingResults = false;
        emit awaitingResultsChanged();
    }

    foreach (const QFileInfo &fi, newFiles) {
        if (!mShowDirectories && fi.isDir())
            continue;

        bool doAdd = true;
        foreach (const QString &nameFilter, mNameFilters) {
            // TODO: using QRegExp for wildcard matching is slow
            QRegExp re(nameFilter, Qt::CaseInsensitive, QRegExp::Wildcard);
            if (!re.exactMatch(fi.fileName())) {
                doAdd = false;
                break;
            }
        }

        if (!doAdd)
            continue;

        QVector<QFileInfo>::Iterator it = qLowerBound(mDirectoryContents.begin(),
                                                      mDirectoryContents.end(),
                                                      fi,
                                                      fileCompare);

        if (it == mDirectoryContents.end()) {
            beginInsertRows(QModelIndex(), mDirectoryContents.count(), mDirectoryContents.count());
            mDirectoryContents.append(fi);
            endInsertRows();
        } else {
            int idx = it - mDirectoryContents.begin();
            beginInsertRows(QModelIndex(), idx, idx);
            mDirectoryContents.insert(it, fi);
            endInsertRows();
        }
    }
}

void DirModel::rm(const QStringList &paths)
{
    // TODO: handle directory deletions?
    bool error = false;

    foreach (const QString &path, paths) {
        error |= QFile::remove(path);

        if (error) {
            qWarning() << Q_FUNC_INFO << "Failed to remove " << path;
            error = false;
        }
    }

    // TODO: just remove removed items; don't reload the entire model
    refresh();
}

bool DirModel::rename(int row, const QString &newName)
{
    qDebug() << Q_FUNC_INFO << "Renaming " << row << " to " << newName;
    Q_ASSERT(row >= 0 && row < mDirectoryContents.count());
    if (row < 0 || row >= mDirectoryContents.count()) {
        qWarning() << Q_FUNC_INFO << "Out of bounds access";
        return false;
    }

    const QFileInfo &fi = mDirectoryContents.at(row);

    if (!fi.isDir()) {
        QFile f(fi.absoluteFilePath());
        bool retval = f.rename(fi.absolutePath() + QDir::separator() + newName);

        if (!retval)
            qDebug() << Q_FUNC_INFO << "Rename returned error code: " << f.error() << f.errorString();
        else
            refresh();
        // TODO: just change the affected item... ^^

        return retval;
    } else {
        QDir d(fi.absoluteFilePath());
        bool retval = d.rename(fi.absoluteFilePath(), fi.absolutePath() + QDir::separator() + newName);

        // QDir has no way to detect what went wrong. woohoo!

        // TODO: just change the affected item...
        refresh();

        return retval;
    }

    // unreachable (we hope)
    Q_ASSERT(false);
    return false;
}

void DirModel::mkdir(const QString &newDir)
{
    qDebug() << Q_FUNC_INFO << "Creating new folder " << newDir << " to " << mCurrentDir;

    QDir dir(mCurrentDir);
    bool retval = dir.mkdir(newDir);
    if (!retval) {
        const char *errorStr = strerror(errno);
        qDebug() << Q_FUNC_INFO << "Error creating new directory: " << errno << " (" << errorStr << ")";
        emit error("Error creating new folder", errorStr);
    } else {
        refresh();
    }
}

bool DirModel::showDirectories() const
{
    return mShowDirectories;
}

void DirModel::setShowDirectories(bool showDirectories)
{
    mShowDirectories = showDirectories;
    refresh();
    emit showDirectoriesChanged();
}

QStringList DirModel::nameFilters() const
{
    return mNameFilters;
}

void DirModel::setNameFilters(const QStringList &nameFilters)
{
    mNameFilters = nameFilters;
    refresh();
    emit nameFiltersChanged();
}

bool DirModel::awaitingResults() const
{
    return mAwaitingResults;
}

// for dirlistworker
#include "dirmodel.moc"
