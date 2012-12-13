include(../src/src.pro)
SRCDIR = ../../src/
INCLUDEPATH += $$SRCDIR
DEPENDPATH = $$INCLUDEPATH
QT += testlib
TEMPLATE = app
CONFIG -= app_bundle

CONFIG += link_pkgconfig
PKGCONFIG += \
    accounts-qt \
    QtDeclarative

target.path = /opt/tests/nemo-qml-plugins/accounts
INSTALLS += target
