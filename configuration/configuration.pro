TARGET = nemoconfiguration
PLUGIN_IMPORT_PATH = org/nemomobile/configuration

SOURCES += plugin.cpp \
           configurationvalue.cpp

HEADERS += dirmodel.h \
           configurationvalue.h

CONFIG += link_pkgconfig
PKGCONFIG += mlite

include(../plugin.pri)
