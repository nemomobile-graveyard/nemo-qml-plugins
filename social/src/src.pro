TARGET = nemosocial
PLUGIN_IMPORT_PATH = org/nemomobile/social

QT += declarative network

lessThan(QT_MAJOR_VERSION, 5) {
    CONFIG += link_pkgconfig
    PKGCONFIG += QJson
}

include(facebook/facebook.pri)

SOURCES += \
    $$PWD/plugin.cpp \
    $$PWD/contentiteminterface.cpp \
    $$PWD/filterinterfaces.cpp \
    $$PWD/identifiablecontentiteminterface.cpp \
    $$PWD/socialnetworkinterface.cpp \
    $$PWD/sorterinterfaces.cpp

HEADERS += \
    $$PWD/contentiteminterface.h \
    $$PWD/contentiteminterface_p.h \
    $$PWD/contentitemtypefilterinterface.h \
    $$PWD/filterinterface.h \
    $$PWD/identifiablecontentiteminterface.h \
    $$PWD/identifiablecontentiteminterface_p.h \
    $$PWD/socialnetworkinterface.h \
    $$PWD/socialnetworkinterface_p.h \
    $$PWD/sorterinterface.h

OTHER_FILES += qmldir social.qdoc social.qdocconf

MOC_DIR = $$PWD/../.moc
OBJECTS_DIR = $$PWD/../.obj

include(../../plugin.pri)
