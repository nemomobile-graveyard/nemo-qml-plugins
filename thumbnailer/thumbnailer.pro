TARGET = nemothumbnailer
PLUGIN_IMPORT_PATH = org/nemomobile/thumbnailer

SOURCES += plugin.cpp \
           nemothumbnailprovider.cpp \
           nemoimagemetadata.cpp
HEADERS += nemothumbnailprovider.h \
           nemoimagemetadata.h

include(../plugin.pri)
