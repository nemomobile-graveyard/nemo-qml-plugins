TARGET = nemoconfiguration
PLUGIN_IMPORT_PATH = org/nemomobile/configuration

SOURCES += plugin.cpp \
           configurationvalue.cpp

HEADERS += dirmodel.h \
           configurationvalue.h

CONFIG += link_pkgconfig
PKGCONFIG += mlite

# do not edit below here, move this to a shared .pri?
TEMPLATE = lib
CONFIG += qt plugin hide_symbols
QT += declarative

target.path = $$[QT_INSTALL_IMPORTS]/$$PLUGIN_IMPORT_PATH
INSTALLS += target

qmldir.files += $$PWD/qmldir
qmldir.path +=  $$[QT_INSTALL_IMPORTS]/$$$$PLUGIN_IMPORT_PATH
INSTALLS += qmldir
