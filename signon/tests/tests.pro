TEMPLATE = subdirs
SUBDIRS = tst_identityinterface

tests_xml.target = tests.xml
tests_xml.files = tests.xml
tests_xml.path = /opt/tests/nemo-qml-plugins/signon
INSTALLS += tests_xml
