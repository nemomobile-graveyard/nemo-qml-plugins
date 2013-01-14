TARGET = nemoalarms
PLUGIN_IMPORT_PATH = org/nemomobile/alarms

QT += dbus

CONFIG += link_pkgconfig
PKGCONFIG += timed timed-voland

SOURCES += plugin.cpp \
    alarmsbackendmodel.cpp \
    alarmsbackendmodel_p.cpp \
    alarmobject.cpp \
    alarmhandlerinterface.cpp \
    alarmdialogobject.cpp

HEADERS += alarmsbackendmodel.h \
    alarmsbackendmodel_p.h \
    alarmobject.h \
    alarmhandlerinterface.h \
    alarmdialogobject.h

include(../../plugin.pri)
