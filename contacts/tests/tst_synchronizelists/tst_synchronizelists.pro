QT += testlib
TEMPLATE = app
CONFIG -= app_bundle

SRCDIR = $$PWD/../../src

CONFIG += mobility
MOBILITY += contacts


target.path = /opt/tests/nemo-qml-plugins/contacts
INSTALLS += target

INCLUDEPATH += $$SRCDIR

HEADERS += \
        $$SRCDIR/synchronizelists_p.h

SOURCES += \
        tst_synchronizelists.cpp
