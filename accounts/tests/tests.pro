TEMPLATE = subdirs
SUBDIRS = \
    tst_accountinterface \
    tst_accountmanagerinterface \
    tst_providerinterface \
    tst_serviceinterface \
    tst_servicetypeinterface \
    tst_serviceaccountinterface

tests_xml.target = tests.xml
tests_xml.files = tests.xml
tests_xml.path = /opt/tests/nemo-qml-plugins/accounts
INSTALLS += tests_xml

tests_provider.target = test-provider.provider
tests_provider.files = test-provider.provider
tests_provider.path = /usr/share/accounts/providers
INSTALLS += tests_provider

tests_service.target = test-service2.service
tests_service.files = test-service2.service
tests_service.path = /usr/share/accounts/services
INSTALLS += tests_service

tests_service_type.target = test-service-type2.service-type
tests_service_type.files = test-service-type2.service-type
tests_service_type.path = /usr/share/accounts/service_types
INSTALLS += tests_service_type
