TARGET = nemotime
PLUGIN_IMPORT_PATH = org/nemomobile/time

SOURCES += plugin.cpp \
           nemowallclock.cpp

HEADERS += nemowallclock.h \
            nemowallclock_p.h

LIBS += -ltimed

QT += dbus
SOURCES += nemowallclock_meego.cpp
#    SOURCES += nemowallclock_generic.cpp

include(../../plugin.pri)
