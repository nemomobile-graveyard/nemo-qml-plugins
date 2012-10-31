TARGET = nemocontacts
PLUGIN_IMPORT_PATH = org/nemomobile/contacts

INSTALL_PREFIX = /usr

CONFIG += link_pkgconfig
packagesExist(icu-i18n) {
    DEFINES += HAS_ICU
    PKGCONFIG += icu-i18n
} else {
    warning("ICU not detected. This may cause problems with i18n.")
}

CONFIG += mobility
MOBILITY += contacts versit

PUBLIC_HEADERS += $$PWD/seasideperson.h \
                  $$PWD/seasidepeoplemodel.h \
                  $$PWD/seasideproxymodel.h

PRIVATE_HEADERS += $$PWD/localeutils_p.h \
                   $$PWD/seasidepeoplemodel_p.h

SOURCES += $$PWD/plugin.cpp \
           $$PWD/localeutils.cpp \
           $$PWD/seasidepeoplemodel.cpp \
           $$PWD/seasidepeoplemodel_p.cpp \
           $$PWD/seasideperson.cpp \
           $$PWD/seasideproxymodel.cpp

HEADERS += $$PUBLIC_HEADERS \
           $$PRIVATE_HEADERS

MOC_DIR = $$PWD/../.moc
OBJECTS_DIR = $$PWD/../.obj

include(../../plugin.pri)

lib.path = $${INSTALL_PREFIX}/lib
lib.files = libnemocontacts.so

headers.path = $${INSTALL_PREFIX}/include/nemo-qml-plugins/contacts
headers.files = $${PUBLIC_HEADERS}

pkgconfig.files = contacts.pc
pkgconfig.path = $${INSTALL_PREFIX}/lib/pkgconfig

INSTALLS += lib \
            headers \
            pkgconfig
