//project
#include "account-provider-model.h"
#include "provider-helper.h"

//Qt
#include <QDebug>
#include <QStringList>
#include <QDir>
#include <QPointer>
#include <QCoreApplication>
#include <QMap>
#include <QtAlgorithms>

//libaccounts-qt
#include <Accounts/Manager>

class AccountProviderModel::AccountProviderModelPrivate
{
public:
    ~AccountProviderModelPrivate()
    {
        qDeleteAll(providerList);
    }

    QList<ProviderHelper *> providerList;
    QHash<int, QByteArray> headerData;
    Accounts::Manager *manager;
};

AccountProviderModel::AccountProviderModel(QObject* parent)
    : QAbstractTableModel(parent)
    , d_ptr(new AccountProviderModelPrivate())
{
    Q_D(AccountProviderModel);

    d->headerData.insert(ProviderNameRole, "providerName");
    d->headerData.insert(ProviderDescriptionRole, "providerDescription" );
    d->headerData.insert(ProviderIconRole, "providerIcon");
    d->headerData.insert(ProviderRole, "provider");
    d->headerData.insert(ColumnCountRole, "columncount");

    setRoleNames(d->headerData);
    Accounts::Manager m;
    Accounts::ProviderList providers = m.providerList();

    for (int i = 0; i < providers.size(); i++)
    {
        QDomDocument domDocument = providers[i].domDocument();
        d->providerList << new ProviderHelper(providers[i]);
    }
}

QVariant AccountProviderModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_D(const AccountProviderModel);

    if (orientation != Qt::Horizontal) {
        return QVariant();
    }

    Columns column = static_cast<Columns>(section);

    if (role == Qt::DisplayRole) {
        if (section < d->headerData.size()) {
            return d->headerData.value(column);
        }
    }
    return QVariant();
}


AccountProviderModel::~AccountProviderModel()
{
    Q_D(AccountProviderModel);

    delete d;
}

QDomDocument AccountProviderModel::domDocument(const QModelIndex& currentIndex)
{
    Q_D(AccountProviderModel);
    ProviderHelper *providerHelper;

    providerHelper = d->providerList.at(currentIndex.row());
    if (!providerHelper)
        return QDomDocument();

    return providerHelper->domDocument();
}


QModelIndex AccountProviderModel::index(int row, int column, const QModelIndex& parent) const
{

    if (row < 0 || column < 0 || !hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

int AccountProviderModel::rowCount(const QModelIndex& parent) const
{
    Q_D(const AccountProviderModel);
    if (parent.isValid())
        return 0;
    return d->providerList.count();
}

int AccountProviderModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return ColumnCount;
}

QVariant AccountProviderModel::data(const QModelIndex& index, int role) const
{
    Q_D(const AccountProviderModel);
    if (!index.isValid())
        return QVariant();
    Q_UNUSED(role);

    ProviderHelper *providerHelper = d->providerList.at(index.row());
    if (!providerHelper)
        return QVariant();

    if (role == ProviderNameRole ||
            (role == Qt::DisplayRole && index.column() == ProviderNameColumn))
        return providerHelper->providerName();

    if (role == ProviderDescriptionRole ||
        (role == Qt::DisplayRole && index.column() == ProviderDescriptionColumn))
        return providerHelper->providerDescription();

    if (role == ProviderIconRole ||
            (role == Qt::DisplayRole && index.column() == ProviderIconColumn))
        return providerHelper->iconName();

    if (role == ProviderRole ||
            (role == Qt::DisplayRole && index.column() == ProviderColumn))
        return QVariant::fromValue(providerHelper->provider());

    return QVariant();
}


Q_DECLARE_METATYPE(Accounts::Provider)
