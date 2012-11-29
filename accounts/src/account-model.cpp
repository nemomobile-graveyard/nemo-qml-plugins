#include <QDebug>

#include "account-model.h"
#include "provider-helper.h"

//libaccounts-qt
#include <Accounts/Manager>
#include <Accounts/Account>
#include <Accounts/Provider>

class AccountModel::AccountModelPrivate
{
public:
    ~AccountModelPrivate()
    {
        qDeleteAll(accountsList);
    }

    QHash<int, QByteArray> headerData;
    Accounts::Manager *manager;
    QList<DisplayData *> accountsList;
};

AccountModel::AccountModel(QObject* parent)
    : QAbstractListModel(parent)
    , d_ptr(new AccountModelPrivate())
{
    Q_D(AccountModel);
    d->manager = new Accounts::Manager();
    d->headerData.insert(AccountRole, "account");
    d->headerData.insert(AccountNameRole, "accountName");
    d->headerData.insert(AccountIconRole, "accountIcon" );
    d->headerData.insert(ProviderNameRole, "providerName");
    d->headerData.insert(ColumnCountRole, "columncount");
    QObject::connect(d->manager, SIGNAL(accountCreated(Accounts::AccountId)),
                     this, SLOT(accountCreated(Accounts::AccountId)));
    setRoleNames(d->headerData);
    Accounts::AccountIdList idList = d->manager->accountList();
    foreach (Accounts::AccountId id, idList)
    {
        Accounts::Account *account = d->manager->account(id);
        d->accountsList.append(new DisplayData(account));
    }
}

AccountModel::~AccountModel()
{
}

int AccountModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const AccountModel);
    if (parent.isValid())
        return 0;
    return d->accountsList.length();
}

int AccountModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return ColumnCount;
}

QVariant AccountModel::data(const QModelIndex &index, int role) const
{
    Q_D(const AccountModel);
    if (!index.isValid())
        return QVariant();

    DisplayData *data = d->accountsList[index.row()];
    Accounts::Account *account = data->account;

    if (role == AccountRole ||
            (role == Qt::DisplayRole && index.column() == AccountColumn))
        return QVariant::fromValue(account);

    if (role == AccountNameRole ||
            (role == Qt::DisplayRole && index.column() == AccountNameColumn))
        return QVariant::fromValue(account->displayName());

    if (role == AccountIconRole ||
            (role == Qt::DisplayRole && index.column() == AccountIconColumn)) {
        if (data->accountIcon.isNull()) {
            Accounts::Provider provider =
                d->manager->provider(account->providerName());
            ProviderHelper helper(provider);
            data->accountIcon = helper.iconName();
        }
        return QVariant::fromValue(data->accountIcon);
    }

    if (role == ProviderNameRole ||
            (role == Qt::DisplayRole && index.column() == ProviderNameColumn)) {
        Accounts::Provider provider = d->manager->provider(account->providerName());
        if (provider.isValid()) {
            ProviderHelper helper(provider);
            QString providerName = helper.providerName();
            data->providerName = providerName;
        }
        return QVariant::fromValue(data->providerName);
    }
    return QVariant();
}

QVariant AccountModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_D(const AccountModel);
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

void AccountModel::accountCreated(Accounts::AccountId id)
{
    Q_D(AccountModel);
    QModelIndex index;
    Accounts::Account *account = d->manager->account(id);

    QObject::connect(account, SIGNAL(removed()),
                     this, SLOT(accountRemoved()));
    QObject::connect(account, SIGNAL(enabledChanged(const QString, bool)),
                     this, SLOT(accountUpdated()));

    if (account != 0) {
        beginInsertRows(index, 0, 0);
        d->accountsList.insert(0, new DisplayData(account));
        endInsertRows();
    }
}

void AccountModel::accountRemoved()
{
    Q_D(AccountModel);
    Accounts::Account *account = qobject_cast<Accounts::Account *>(sender());

    /* find the position of the deleted account in the list: QAbstractItemModel
     * APIs need it */
    int index = getAccountIndex(account);
    if (index < 0)
    {
        qDebug() << "Account not present in the list:" << account->id();
        return;
    }

    QModelIndex parent;
    beginRemoveRows(parent, index, index);
    DisplayData *data = d->accountsList[index];
    d->accountsList.removeAt(index);
    endRemoveRows();

    delete data;
}

void AccountModel::accountUpdated()
{
    Accounts::Account *account = qobject_cast<Accounts::Account *>(sender());
    int accountIndex = getAccountIndex(account);
    if (accountIndex < 0)
    {
        qDebug() << "Account not present in the list:" << account->id();
        return;
    }

    emit dataChanged(index(accountIndex, 0), index(accountIndex, 0));
}

int AccountModel::getAccountIndex(Accounts::Account *account) const
{
    Q_D(const AccountModel);
    int index;
    for (index = 0; index < d->accountsList.count(); index ++) {
        const DisplayData *data = d->accountsList[index];
        if (data->account == account)
            break;
    }
    return index < d->accountsList.count() ? index : -1;
}

