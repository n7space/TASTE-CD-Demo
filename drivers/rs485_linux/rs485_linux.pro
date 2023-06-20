TEMPLATE = lib
CONFIG -= qt
CONFIG += generateC

DISTFILES += interfaceview.xml
DISTFILES += rs485_linux.asn
DISTFILES += rs485_linux.acn
DISTFILES += rs485_linuxconfiguration.asn
DISTFILES += rs485_linuxconfiguration.acn
DISTFILES += rs485_linuxprivatedata.asn
DISTFILES += rs485_linuxprivatedata.acn

include(work/taste.pro)
message($$DISTFILES)