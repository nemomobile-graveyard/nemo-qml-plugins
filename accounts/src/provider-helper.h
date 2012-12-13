#ifndef PROVIDERHELPER_H
#define PROVIDERHELPER_H

//Qt
#include <QImage>
#include <QDomDocument>

//accounts-qt
#include <Accounts/Provider>
#include <Accounts/Account>

/*!
 * ProviderHelper class that takes the provider data and help in contructing UIs
 */
class ProviderHelper : public QObject
{
    Q_OBJECT
    class ProviderHelperPrivate;

    typedef Accounts::Provider Provider;

public:
    explicit ProviderHelper(Provider provider, QObject *parent = 0);
    virtual ~ProviderHelper();

    /*!
     * Fetches the QDomDocument installed by the provider.
     */
    const QDomDocument domDocument() const;

    /*!
     * Fetches the display name of the provider.
     */
    QString providerName();

    /*!
     * Fetches the display icon of the provider.
     */
    QString iconName();

    /*!
     * Fetches the provider description.
     */
    QString providerDescription();

    /*!
     * Fetches the provider object.
     */
    Accounts::Provider provider() ;

private:
    ProviderHelperPrivate *d_ptr;
    Q_DECLARE_PRIVATE(ProviderHelper)
};

#endif // PROVIDERHELPER_H
