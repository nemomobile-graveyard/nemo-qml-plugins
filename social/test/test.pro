TARGET = socialtest
TARGETPATH = /opt/tests/nemo-qml-plugins/social/
target.path = $$TARGETPATH

DEPLOYMENT_PATH = /opt/tests/nemo-qml-plugins/social/
qml.files = *qml
qml.path = $$DEPLOYMENT_PATH

QT += declarative
contains(CONFIG, desktop) {
   DEFINES += DESKTOP
   QT += opengl
}

SOURCES += main.cpp
OTHER_FILES += *qml

!contains(CONFIG, desktop) {
    INSTALLS += target qml desktop
}

DEFINES *= DEPLOYMENT_PATH=\"\\\"\"$${DEPLOYMENT_PATH}/\"\\\"\"

CONFIG += link_pkgconfig
packagesExist(qdeclarative-boostable) {
    message("Building with qdeclarative-boostable support")
    DEFINES += HAS_BOOSTER
    PKGCONFIG += qdeclarative-boostable
} else {
    warning("qdeclarative-boostable not available; startup times will be slower")
}
