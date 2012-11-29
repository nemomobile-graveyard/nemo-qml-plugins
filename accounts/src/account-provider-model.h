#ifndef ACCOUNTPROVIDERMODEL_H
#define ACCOUNTPROVIDERMODEL_H

//project
#include "provider-helper.h"

//accounts-qt
#include<Accounts/Manager>

//Qt
#include <QAbstractTableModel>
#include <QDomDocument>

/*!
 * The AccountProviderModel is a model that creates list of available providers.
 * It fetches the provider xml and creates the list view
 */
class AccountProviderModel : public QAbstractTableModel
{
    Q_OBJECT
    class AccountProviderModelPrivate;

public:

    enum Columns{
        ProviderNameColumn,
        ProviderDescriptionColumn,
        ProviderIconColumn,
        ProviderColumn,
        ColumnCount
    };

    enum Roles{
        ProviderNameRole = Qt::UserRole + 1,
        ProviderDescriptionRole,
        ProviderIconRole,
        ProviderRole,
        ColumnCountRole
    };

    /*!
     * The Constructor
     */
    AccountProviderModel(QObject* parent);

    /*!
     * The Destructor.
     */
    ~AccountProviderModel();

    /*!
     * To get the domDocument of the provider xml
     */
    QDomDocument domDocument(const QModelIndex& index);

    /*!
     *  Derived from QAbstractListModel - gives the number of rows in the model.
     */
    int rowCount( const QModelIndex & index = QModelIndex() ) const;

    /*!
     *  Derived from QAbstractListModel - gives the number of columns in the model.
     */
    int columnCount( const QModelIndex & index ) const;

    /*!
     *  Fetches data depending on the index and the role provided.
     */
    QVariant data( const QModelIndex &index, int role ) const;

    /*!
     *  Derived from QAbstractTableModel - provides QModelIndex
     */
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;

    /*!
    *  Maps all the roles with a QString so that qmlList can access the model
    */
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    AccountProviderModelPrivate* d_ptr;
    Q_DISABLE_COPY(AccountProviderModel)
    Q_DECLARE_PRIVATE(AccountProviderModel);
};

#endif // ACCOUNTPROVIDERMODEL_H
