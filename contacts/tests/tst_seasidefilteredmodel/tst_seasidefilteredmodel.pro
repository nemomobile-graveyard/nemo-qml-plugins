QT += testlib
TEMPLATE = app
CONFIG -= app_bundle

SRCDIR = $$PWD/../../src

CONFIG += mobility
MOBILITY += contacts

QT += declarative

target.path = /opt/tests/nemo-qml-plugins/contacts
INSTALLS += target

INCLUDEPATH += $$SRCDIR
HEADERS += \
        seasidecache.h \
        $$SRCDIR/seasidefilteredmodel.h \
        $$SRCDIR/seasideperson.h \
        $$SRCDIR/synchronizedlists_p.h

SOURCES += \
        tst_seasidefilteredmodel.cpp \
        $$SRCDIR/seasidefilteredmodel.cpp \
        $$SRCDIR/seasideperson.cpp \
        seasidecache.cpp
