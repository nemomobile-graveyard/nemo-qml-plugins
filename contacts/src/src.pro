TARGET = nemocontacts
PLUGIN_IMPORT_PATH = org/nemomobile/contacts

CONFIG += link_pkgconfig
packagesExist(icu-i18n) {
    DEFINES += HAS_ICU
    PKGCONFIG += icu-i18n
} else {
    warning("ICU not detected. This may cause problems with i18n.")
}

packagesExist(mlite) {
    PKGCONFIG += mlite
    DEFINES += HAS_MLITE
} else {
    warning("mlite not available. Some functionality may not work as expected.")
}

CONFIG += mobility
MOBILITY += contacts versit

SOURCES += $$PWD/plugin.cpp \
           $$PWD/localeutils.cpp \
           $$PWD/seasidepeoplemodel.cpp \
           $$PWD/seasidepeoplemodel_p.cpp \
           $$PWD/seasideperson.cpp \
           $$PWD/seasideproxymodel.cpp \
           $$PWD/seasidecache.cpp \
           $$PWD/seasidefilteredmodel.cpp

HEADERS += $$PWD/localeutils_p.h \
           $$PWD/synchronizelists_p.h \
           $$PWD/seasidepeoplemodel.h \
           $$PWD/seasidepeoplemodel_p.h \
           $$PWD/seasideperson.h \
           $$PWD/seasideproxymodel.h \
           $$PWD/seasidecache.h \
           $$PWD/seasidefilteredmodel.h

MOC_DIR = $$PWD/../.moc
OBJECTS_DIR = $$PWD/../.obj

include(../../plugin.pri)
