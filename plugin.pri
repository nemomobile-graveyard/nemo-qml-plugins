TEMPLATE = lib
CONFIG += qt plugin hide_symbols
QT += declarative
CONFIG += mobility
MOBILITY += contacts versit

target.path = $$[QT_INSTALL_IMPORTS]/$$PLUGIN_IMPORT_PATH
INSTALLS += target

qmldir.files += $$PWD/qmldir
qmldir.path +=  $$[QT_INSTALL_IMPORTS]/$$$$PLUGIN_IMPORT_PATH
INSTALLS += qmldir

