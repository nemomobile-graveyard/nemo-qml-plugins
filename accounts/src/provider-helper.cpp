
//project
#include "provider-helper.h"

//Qt
#include <QDir>
#include <QDebug>
#include <QPointer>
#include <QFile>

using namespace Accounts;

class ProviderHelper::ProviderHelperPrivate
{
public:
    ProviderHelperPrivate(Provider p) :
        provider(p)
    {
        domDocument = provider.domDocument();
    }

    QDomDocument domDocument;
    Accounts::Provider provider;
};


ProviderHelper::ProviderHelper(Provider provider, QObject *parent)
    : QObject(parent),
    d_ptr(new ProviderHelperPrivate(provider))
{
}

ProviderHelper::~ProviderHelper()
{
    delete d_ptr;
}

const QDomDocument ProviderHelper::domDocument() const
{
    Q_D(const ProviderHelper);
    return d->provider.domDocument();
}

QString ProviderHelper::providerName()
{
    Q_D(ProviderHelper);
    return d->provider.displayName();
}

QString ProviderHelper::iconName()
{
    Q_D(ProviderHelper);
    return d->provider.iconName();
}

QString ProviderHelper::providerDescription()
{
    Q_D(ProviderHelper);
    QDomElement root = d->domDocument.documentElement();
    QDomElement descriptionElement = root.firstChildElement("description");
    if (!descriptionElement.text().isEmpty())
        return descriptionElement.text();
    else
        return QString("no description found");
}

Accounts::Provider ProviderHelper::provider()
{
    Q_D(ProviderHelper);
    return d->provider;
}

