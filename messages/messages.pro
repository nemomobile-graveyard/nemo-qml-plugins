TARGET = nemomessages
PLUGIN_IMPORT_PATH = org/nemomobile/messages
VERSION = $$PROJECT_VERSION

HEADERS += src/messagesmanager.h
SOURCES += src/plugin.cpp \
    src/messagesmanager.cpp

QT += dbus

include(../plugin.pri)
