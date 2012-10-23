include(../src/src.pro)
SRCDIR = ../../src/
INCLUDEPATH += $$SRCDIR
DEPENDPATH = $$INCLUDEPATH
QT += testlib
TEMPLATE = app
CONFIG -= app_bundle

CONFIG += mobility
MOBILITY += contacts versit

target.path = /opt/tests/nemo-qml-plugins/contacts
INSTALLS += target
