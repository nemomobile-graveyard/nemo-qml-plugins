TEMPLATE = subdirs
CONFIG = ordered

SUBDIRS += \
    examples \
    src

OTHER_FILES = qmldir

qmldir.files = qmldir
qmldir.path = /usr/lib/qt4/imports/stage/rubyx/QREncode

INSTALLS += qmldir
