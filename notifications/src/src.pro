system(qdbusxml2cpp org.freedesktop.Notifications.xml -p notificationmanagerproxy -c NotificationManagerProxy -i notification.h)

TARGET = nemonotifications
PLUGIN_IMPORT_PATH = org/nemomobile/notifications
QT += dbus

SOURCES += plugin.cpp \
    notification.cpp \
    notificationmanagerproxy.cpp

HEADERS += \
    notification.h \
    notificationmanagerproxy.h

OTHER_FILES += qmldir notifications.qdoc notifications.qdocconf

include(../../plugin.pri)

headers.files = notification.h
headers.path = /usr/include/nemo-qml-plugins/notifications
pkgconfig.files = nemonotifications.pc
pkgconfig.path = /usr/lib/pkgconfig
INSTALLS += headers pkgconfig
