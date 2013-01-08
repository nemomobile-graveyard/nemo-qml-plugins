include(../common.pri)
TARGET = tst_wallclock

SOURCES += tst_wallclock.cpp

tests_qml.target = update.qml
tests_qml.files = update.qml
tests_qml.path = /opt/tests/nemo-qml-plugins/time
INSTALLS += tests_qml

