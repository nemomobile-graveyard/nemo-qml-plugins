TARGET = nemothumbnailer
PLUGIN_IMPORT_PATH = org/nemomobile/thumbnailer

SOURCES += plugin.cpp \
           nemothumbnailprovider.cpp \
           nemoimagemetadata.cpp \
           nemothumbnailitem.cpp    \
           nemovideothumbnailer.cpp
HEADERS += nemothumbnailprovider.h \
           nemoimagemetadata.h \
           nemothumbnailitem.h \
           nemovideothumbnailer.h

CONFIG += link_pkgconfig
GSTREAMER_PACKAGES = \
        gstreamer-0.10 \
        gstreamer-app-0.10

packagesExist($$GSTREAMER_PACKAGES) {
    PKGCONFIG += $$GSTREAMER_PACKAGES
    DEFINES *= NEMO_GSTREAMER_THUMBNAILS
}

include(../plugin.pri)
