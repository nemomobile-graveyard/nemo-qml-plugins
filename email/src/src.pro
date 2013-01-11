TARGET = nemoemail
PLUGIN_IMPORT_PATH = org/nemomobile/email
QT += webkit network
CONFIG += link_pkgconfig \

PKGCONFIG += qmfmessageserver \
    qmfclient

packagesExist(mlite) {
    PKGCONFIG += mlite
    DEFINES += HAS_MLITE
} else {
    warning("mlite not available. Some functionality may not work as expected.")
}

SOURCES += \
    $$PWD/emailaccountlistmodel.cpp \
    $$PWD/emailmessagelistmodel.cpp \
    $$PWD/folderlistmodel.cpp \
    $$PWD/emailagent.cpp \
    $$PWD/emailmessage.cpp \
    $$PWD/emailaccountsettingsmodel.cpp \
    $$PWD/emailaccount.cpp \
    $$PWD/htmlfield.cpp \
    $$PWD/plugin.cpp \
    emailaction.cpp

HEADERS += \
    $$PWD/emailaccountlistmodel.h \
    $$PWD/emailmessagelistmodel.h \
    $$PWD/folderlistmodel.h \
    $$PWD/emailagent.h \
    $$PWD/emailmessage.h \
    $$PWD/emailaccountsettingsmodel.h \
    $$PWD/emailaccount.h \
    $$PWD/htmlfield.h \
    $$PWD/emailaction.h

MOC_DIR = $$PWD/../.moc
OBJECTS_DIR = $$PWD/../.obj

include(../../plugin.pri)
