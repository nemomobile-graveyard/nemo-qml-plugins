TEMPLATE = subdirs
SUBDIRS = tst_seasideproxymodel \
          tst_seasideperson \
          tst_seasidefilteredmodel \
          tst_synchronizelists

tests_xml.target = tests.xml
tests_xml.files = tests.xml
tests_xml.path = /opt/tests/nemo-qml-plugins/contacts
INSTALLS += tests_xml
