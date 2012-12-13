TARGET = nemoaccounts
PLUGIN_IMPORT_PATH = org/nemomobile/accounts

QT += declarative

CONFIG += link_pkgconfig
packagesExist(icu-i18n) {
    DEFINES += HAS_ICU
    PKGCONFIG += icu-i18n
} else {
    warning("ICU not detected. This may cause problems with i18n.")
}

PKGCONFIG += \
    accounts-qt \
    QtDeclarative

SOURCES += \
           $$PWD/accountinterface.cpp \
           $$PWD/accountmanagerinterface.cpp \
           $$PWD/account-model.cpp \
           $$PWD/account-provider-model.cpp \
           $$PWD/authdatainterface.cpp \
           $$PWD/plugin.cpp \
           $$PWD/provider-helper.cpp \
           $$PWD/providerinterface.cpp \
           $$PWD/serviceaccountinterface.cpp \
           $$PWD/serviceinterface.cpp \
           $$PWD/servicetypeinterface.cpp

HEADERS += \
           $$PWD/accountinterface.h \
           $$PWD/accountmanagerinterface.h \
           $$PWD/account-model.h \
           $$PWD/account-provider-model.h \
           $$PWD/authdatainterface.h \
           $$PWD/provider-helper.h \
           $$PWD/providerinterface.h \
           $$PWD/serviceaccountinterface.h \
           $$PWD/serviceinterface.h \
           $$PWD/accountinterface_p.h \
           $$PWD/accountmanagerinterface_p.h \
           $$PWD/serviceaccountinterface_p.h \
           $$PWD/servicetypeinterface.h

OTHER_FILES += accounts.qdoc accounts.qdocconf

MOC_DIR = $$PWD/../.moc
OBJECTS_DIR = $$PWD/../.obj

include(../../plugin.pri)
