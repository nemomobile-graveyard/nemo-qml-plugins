TEMPLATE = lib
CONFIG += hide_symbols plugin
TARGET = videothumbnailer

target.path = $$[QT_INSTALL_IMPORTS]/org/nemomobile/thumbnailer/thumbnailers
INSTALLS += target

SOURCES += gstvideothumbnailer.cpp

CONFIG += link_pkgconfig
PKGCONFIG += \
        gstreamer-0.10 \
        gstreamer-app-0.10
