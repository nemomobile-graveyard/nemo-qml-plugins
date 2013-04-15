#include <QtCore/QDir>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QtDeclarative/QDeclarativeComponent>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeItem>
#include <QtDeclarative/QDeclarativeView>
#include <QDebug>

//static const char *IMPORT_PATH = "/opt/sdk/tests/nemo-qml-plugins/social/imports";

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QDeclarativeView view;

    if (argc != 2) {
        qWarning() << "usage: socialtest [facebook_access_token]";
        return 1;
    }

    // TODO: manage better the difference between desktop and device
    QString path = QString(DEPLOYMENT_PATH);
    view.engine()->addImportPath(PLUGIN_PATH);
    view.setSource(path + QLatin1String("socialtest.qml"));

    if (view.status() == QDeclarativeView::Error) {
        qWarning() << "Unable to read main qml file";
        return 1;
    }

    view.rootObject()->setProperty("accessToken", QLatin1String(argv[1]));
    view.rootObject()->setProperty("_desktop", true);
    view.setResizeMode(QDeclarativeView::SizeRootObjectToView);

    view.show();

    QObject::connect(view.engine(), SIGNAL(quit()), &app, SLOT(quit()));

    return app.exec();
}

