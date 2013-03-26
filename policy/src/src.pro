TARGET = nemopolicy
PLUGIN_IMPORT_PATH = org/nemomobile/policy

CONFIG += link_pkgconfig

PKGCONFIG += libresourceqt1

SOURCES += \
        plugin.cpp \
        permissions.cpp \
        resource.cpp

HEADERS += \
        permissions.h \
        resource.h

MOC_DIR = $$PWD/../.moc
OBJECTS_DIR = $$PWD/../.obj

include(../../plugin.pri)
