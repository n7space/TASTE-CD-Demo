TEMPLATE = lib
CONFIG -= qt
CONFIG += generateC

DISTFILES +=  $(HOME)/tool-inst/share/taste-types/taste-types.asn \
    deploymentview.dv.xml
DISTFILES += rs485_usage_test.msc
DISTFILES += interfaceview.xml
DISTFILES += work/binaries/*.msc
DISTFILES += work/binaries/coverage/index.html
DISTFILES += work/binaries/filters
DISTFILES += work/system.asn

DISTFILES += rs485_usage_test.asn
DISTFILES += rs485_usage_test.acn
include(work/taste.pro)
message($$DISTFILES)

