TARGET = socialtest
PREFIX = /opt/nemo-qml-plugins/social/tests

QT = core gui declarative

target.path = $${PREFIX}/bin

SOURCES += main.cpp
OTHER_FILES += *.qml

DEFINES *= PLUGIN_PATH=\"\\\"\"$${DEFINES_PREFIX}/$$[QT_INSTALL_IMPORTS]/$$PLUGIN_IMPORT_PATH\"\\\"\"
DEFINES *= DEPLOYMENT_PATH=\"\\\"\"$${DEFINES_PREFIX}/$${PREFIX}/share/\"\\\"\"

qml.files = $${OTHER_FILES}
qml.path = $${PREFIX}/share

INSTALLS += target qml
