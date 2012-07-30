TEMPLATE = subdirs
PLUGIN_IMPORT_PATH = org/nemomobile/qrencode

SUBDIRS += \
    examples \
    src

OTHER_FILES = qmldir

qmldir.files = qmldir
qmldir.path = $$[QT_INSTALL_IMPORTS]/$$PLUGIN_IMPORT_PATH

INSTALLS += qmldir
