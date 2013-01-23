TEMPLATE = subdirs
SUBDIRS = tst_alarmsbackendmodel \
    tst_alarmhandler

tests_xml.target = tests.xml
tests_xml.files = tests.xml
tests_xml.path = /opt/tests/nemo-qml-plugins/alarms
INSTALLS += tests_xml
