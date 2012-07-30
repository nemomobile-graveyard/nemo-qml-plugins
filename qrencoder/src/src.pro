TEMPLATE = lib
TARGET = qrencode-plugin
PLUGIN_IMPORT_PATH = org/nemomobile/qrencode

QT     = core gui declarative
CONFIG = qt plugin link_pkgconfig

packagesExist(libqrencode) {
    PKGCONFIG += libqrencode
} else {
    warning("Dependency libqrencode is missing.")
}

#DEFINES += WANT_DEBUG

contains(MEEGO_EDITION,harmattan) {
    target.path = /opt/src/bin
    INSTALLS += target
}

HEADERS += \
    common.h \
    qrencodeplugin.h \
    qrcodeitem.h

SOURCES += \
    qrencodeplugin.cpp \
    qrcodeitem.cpp

target.path = $$[QT_INSTALL_IMPORTS]/$$PLUGIN_IMPORT_PATH
INSTALLS = target
