TARGET = nemocalendar
PLUGIN_IMPORT_PATH = org/nemomobile/calendar

CONFIG += link_pkgconfig
PKGCONFIG += libkcalcoren libmkcal

SOURCES += \
    plugin.cpp \
    calendarabstractmodel.cpp \
    calendarevent.cpp \
    calendaragendamodel.cpp \
    calendardb.cpp

HEADERS += \
    calendarevent.h \
    calendarabstractmodel.h \
    calendaragendamodel.h \
    calendardb.h

MOC_DIR = $$PWD/../.moc
OBJECTS_DIR = $$PWD/../.obj

include(../../plugin.pri)
