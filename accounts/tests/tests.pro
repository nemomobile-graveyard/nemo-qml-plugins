TEMPLATE = subdirs
SUBDIRS = tst_accountinterface

tests_xml.target = tests.xml
tests_xml.files = tests.xml
tests_xml.path = /opt/tests/nemo-qml-plugins/accounts
INSTALLS += tests_xml

tests_provider.target = test-provider.provider
tests_provider.files = test-provider.provider
tests_provider.path = /usr/share/accounts/providers
INSTALLS += tests_provider

tests_service.target = test-service.service
tests_service.files = test-service.service
tests_service.path = /usr/share/accounts/services
INSTALLS += tests_service
