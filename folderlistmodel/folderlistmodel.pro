TARGET = nemofolderlistmodel
PLUGIN_IMPORT_PATH = org/nemomobile/folderlistmodel

SOURCES += plugin.cpp \
           dirmodel.cpp \
           iorequest.cpp \
           iorequestworker.cpp \
           ioworkerthread.cpp

HEADERS += plugin.h \
           dirmodel.h \
           iorequest.h \
           iorequestworker.h \
           ioworkerthread.h

include(../plugin.pri)
