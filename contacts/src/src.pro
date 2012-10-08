TARGET = nemocontacts
PLUGIN_IMPORT_PATH = org/nemomobile/contacts

CONFIG += link_pkgconfig
packagesExist(icu-i18n) {
    DEFINES += HAS_ICU
} else {
    warning("ICU not detected. This may cause problems with i18n.")
}

SOURCES += $$PWD/plugin.cpp \
           $$PWD/localeutils.cpp \
           $$PWD/seasidepeoplemodel.cpp \
           $$PWD/seasidepeoplemodel_p.cpp \
           $$PWD/seasideperson.cpp \
           $$PWD/seasideproxymodel.cpp

HEADERS += $$PWD/localeutils_p.h \
           $$PWD/seasidepeoplemodel.h \
           $$PWD/seasidepeoplemodel_p.h \
           $$PWD/seasideperson.h \
           $$PWD/seasideproxymodel.h

MOC_DIR = $$PWD/../.moc
OBJECTS_DIR = $$PWD/../.obj

# do not edit below here, move this to a shared .pri?
TEMPLATE = lib
CONFIG += qt plugin hide_symbols
QT += declarative
CONFIG += mobility
MOBILITY += contacts versit

target.path = $$[QT_INSTALL_IMPORTS]/$$PLUGIN_IMPORT_PATH
INSTALLS += target

qmldir.files += $$PWD/qmldir
qmldir.path +=  $$[QT_INSTALL_IMPORTS]/$$$$PLUGIN_IMPORT_PATH
INSTALLS += qmldir
