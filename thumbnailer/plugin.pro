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

DEFINES += NEMO_THUMBNAILER_DIR=\\\"$$[QT_INSTALL_IMPORTS]/$$$$PLUGIN_IMPORT_PATH/thumbnailers\\\"

include(../plugin.pri)
