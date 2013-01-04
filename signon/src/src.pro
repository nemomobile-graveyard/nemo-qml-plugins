TARGET = nemosignon
PLUGIN_IMPORT_PATH = org/nemomobile/signon

QT += declarative

CONFIG += link_pkgconfig
PKGCONFIG += libsignon-qt

SOURCES += \
           $$PWD/identityinterface.cpp \
           $$PWD/identitymanagerinterface.cpp \
           $$PWD/plugin.cpp \
           $$PWD/serviceaccountidentityinterface.cpp \
           $$PWD/sessiondatainterface.cpp

HEADERS += \
           $$PWD/identityinterface.h \
           $$PWD/identityinterface_p.h \
           $$PWD/identitymanagerinterface.h \
           $$PWD/serviceaccountidentityinterface.h \
           $$PWD/serviceaccountidentityinterface_p.h \
           $$PWD/sessiondatainterface.h

OTHER_FILES += signon.qdoc signon.qdocconf

MOC_DIR = $$PWD/../.moc
OBJECTS_DIR = $$PWD/../.obj

include(../../plugin.pri)
