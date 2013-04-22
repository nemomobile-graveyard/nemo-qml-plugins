TARGET = nemoutilities
PLUGIN_IMPORT_PATH = org/nemomobile/utilities

SOURCES += plugin.cpp \
    declarativewindowattributes.cpp \
    declarativescreenshots.cpp

HEADERS += declarativewindowattributes.h \
    declarativescreenshots.h

include(../../plugin.pri)
