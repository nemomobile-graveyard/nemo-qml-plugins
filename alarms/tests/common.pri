include(../src/src.pro)
SRCDIR = ../../src/
INCLUDEPATH += $$SRCDIR
DEPENDPATH = $$INCLUDEPATH
QT += testlib
TEMPLATE = app
CONFIG -= app_bundle

DEFINES += USE_VOLAND_TEST_INTERFACE

CONFIG += link_pkgconfig
PKGCONFIG += \
    QtDeclarative

target.path = /opt/tests/nemo-qml-plugins/alarms
INSTALLS += target
