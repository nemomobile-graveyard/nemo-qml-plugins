TARGET = nemosignon
PLUGIN_IMPORT_PATH = org/nemomobile/signon

QT += declarative

CONFIG += link_pkgconfig
packagesExist(icu-i18n) {
    DEFINES += HAS_ICU
    PKGCONFIG += icu-i18n
} else {
    warning("ICU not detected. This may cause problems with i18n.")
}

PKGCONFIG += \
    libsignon-qt \
    QtDeclarative

SOURCES += \
           $$PWD/authsessioninterface.cpp \
           $$PWD/identityinterface.cpp \
           $$PWD/identitymanagerinterface.cpp \
           $$PWD/plugin.cpp \
           $$PWD/serviceaccountidentityinterface.cpp \
           $$PWD/sessiondatainterface.cpp

HEADERS += \
           $$PWD/authsessioninterface.h \
           $$PWD/authsessioninterface_p.h \
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
