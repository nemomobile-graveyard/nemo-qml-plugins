TARGET = nemoalarms
PLUGIN_IMPORT_PATH = org/nemomobile/alarms

QT += dbus

CONFIG += link_pkgconfig
PKGCONFIG += timed

SOURCES += plugin.cpp \
    alarmsbackendmodel.cpp \
    alarmsbackendmodel_p.cpp \
    alarmobject.cpp

HEADERS += alarmsbackendmodel.h \
    alarmsbackendmodel_p.h \
    alarmobject.h

include(../../plugin.pri)
