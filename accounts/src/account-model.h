#ifndef ACCOUNTMODEL_H
#define ACCOUNTMODEL_H

//accounts-qt
#include <Accounts/Manager>

//Qt
#include <QAbstractTableModel>
#include <QMap>
#include <QVariant>

/*!
 * The Account Model is the model for created accounts
 */

class AccountModel : public QAbstractListModel
{
    Q_OBJECT
    class AccountModelPrivate;

public:

    enum Columns {
        AccountColumn,
        AccountNameColumn,
        AccountIconColumn,
        ProviderNameColumn,
        ColumnCount
    };

    enum Roles{
        AccountRole = Qt::UserRole + 1,
        AccountNameRole,
        AccountIconRole,
        ProviderNameRole,
        ColumnCountRole
    };

    struct DisplayData {
        DisplayData(Accounts::Account *account):
            account(account)
        {
        }
        ~DisplayData() { delete account; }
        Accounts::Account *account;
        QString providerName;
        QString accountIcon;

        Q_DISABLE_COPY(DisplayData);
    };


    AccountModel(QObject *parent = 0);
    virtual ~AccountModel();

    /*!
     * Derived from QAbstractListModel - delivers the number of rows in the
     * model.
     */
    int rowCount(const QModelIndex &index = QModelIndex()) const;

    /*!
     * Derived from QAbstractListModel - returns the number of columns in the
     * model.
     */
    int columnCount(const QModelIndex &index) const;

    /*!
     * Fetches data depending on the index and the role provided.
     */
    QVariant data(const QModelIndex &index, int role) const;

    /*!
    *  Maps all the roles with a QString so that qmlList can access the model
    */
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role) const;

private slots:
    void accountCreated(Accounts::AccountId id);
    void accountRemoved();
    void accountUpdated();

private:
    int getAccountIndex(Accounts::Account *account) const;

private:
    AccountModelPrivate* d_ptr;
    Q_DISABLE_COPY(AccountModel)
    Q_DECLARE_PRIVATE(AccountModel);
};
Q_DECLARE_METATYPE(Accounts::Account *)

#endif // ACCOUNTMODEL_H
