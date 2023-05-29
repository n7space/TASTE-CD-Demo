#!/bin/bash

PROJECT_NAME=$(xmllint --xpath "/*/@projectName" interfaceview.xml | sed -e "s/projectName=//g" | sed -e "s/\"//g")
PARTITION_NAME=$(echo ${PROJECT_NAME,,})

TOOL_INST=$(taste-config --prefix)
CD_INSTALLATION_DIRECTORY=$COMMUNICATION_DEVICE_PATH/SAMV71/port_rs485_seds
FUNCTION_NAMES=$(xmllint --xpath "//*[local-name()='Function']/@name" interfaceview.xml)

DISTFILES=$(qmake rs485_seds.pro -o /tmp/null 2>&1)
ASN1_FILES_WITHOUT_CONFIG=$(find ${DISTFILES} 2>/dev/null | egrep '.asn$$|.asn1$$' | grep -v 'configuration.asn')
ASN1_CONFIG_FILE=$(find ${DISTFILES} 2>/dev/null | egrep '.asn$$|.asn1$$' | grep 'configuration.asn')

mkdir -p $CD_INSTALLATION_DIRECTORY
cp -r snippets/. $CD_INSTALLATION_DIRECTORY
cp rs485_sedsconfiguration.asn $CD_INSTALLATION_DIRECTORY
cp rs485_sedsconfiguration.acn $CD_INSTALLATION_DIRECTORY
mkdir -p $CD_INSTALLATION_DIRECTORY/rs485_seds

ASN1_TYPES_H_FILES=$(find work -name "C_ASN1_Types.h" -not -path 'work/build/*' -a -not -path 'work/dataview/*' -a -not -path 'work/rs485_seds_broker_adapter/*' -a -not -path 'work/packetizer/*' -a -not -path 'work/binaries/*' -a -not -path 'work/Debug/*' -a -not -path 'work/Dump/*' | head -n 1)
ASN1_TYPES_C_FILES=$(find work -name "C_ASN1_Types.c" -not -path 'work/build/*' -a -not -path 'work/dataview/*' -a -not -path 'work/rs485_seds_broker_adapter/*' -a -not -path 'work/packetizer/*' -a -not -path 'work/binaries/*' -a -not -path 'work/Debug/*' -a -not -path 'work/Dump/*' | head -n 1)
H_FILES=$(find work -name "*.h" ! -name "*C_ASN1_Types.h" -not -path 'work/build/*' -a -not -path 'work/dataview/*' -a -not -path 'work/rs485_seds_broker_adapter/*' -a -not -path 'work/packetizer/*' -a -not -path 'work/binaries/*' -a -not -path 'work/Debug/*' -a -not -path 'work/Dump/*')
C_FILES=$(find work -name "*.c" ! -name "*C_ASN1_Types.c" -not -path 'work/build/*' -a -not -path 'work/dataview/*' -a -not -path 'work/rs485_seds_broker_adapter/*' -a -not -path 'work/packetizer/*' -a -not -path 'work/binaries/*' -a -not -path 'work/Debug/*' -a -not -path 'work/Dump/*')
CC_FILES=$(find work -name "*.cc" ! -name "*C_ASN1_Types.c" -not -path 'work/build/*' -a -not -path 'work/dataview/*' -a -not -path 'work/rs485_seds_broker_adapter/*' -a -not -path 'work/packetizer/*' -a -not -path 'work/binaries/*' -a -not -path 'work/Debug/*' -a -not -path 'work/Dump/*')
ADS_FILES=$(find work -name "*.ads" -not -path 'work/build/*' -a -not -path 'work/rs485_seds_broker_adapter/*' -a -not -path 'work/packetizer/*' -a -not -path 'work/binaries/*' -a -not -path 'work/Debug/*' -a -not -path 'work/Dump/*')
ADB_FILES=$(find work -name "*.adb" -not -path 'work/build/*' -a -not -path 'work/rs485_seds_broker_adapter/*' -a -not -path 'work/packetizer/*' -a -not -path 'work/binaries/*' -a -not -path 'work/Debug/*' -a -not -path 'work/Dump/*')
DRIVER_INTERFACE_FILES=$(find work/build/**/${PARTITION_NAME} -name "${PARTITION_NAME}.*")
THREAD_FILES=$(find work/build/**/${PARTITION_NAME} -name "thread_*" -a -not -name "thread_rs485_seds_broker_adapter_receive.*" -a -not -name "thread_packetizer_trigger.*")
INTERFACE_FILES=$(find work/build/**/${PARTITION_NAME} -name "${PARTITION_NAME}_interface.h" -o -name "${PARTITION_NAME}_shared_interface.c")
TRANSPORT_FILES=$(find work/build/**/${PARTITION_NAME} -name "driver_transport.*")
ROUTING_FILES=$(find work/build/**/${PARTITION_NAME} -name "routing.*")
PACKETIZER_ADAPTER_FILES=$(find work/build/packetizer_adapter -name "packetizer_adapter.*")
REQUEST_SIZE_FILE=work/build/**/${PARTITION_NAME}/request_size.h
SYSTEM_CONFIG_FILE=work/build/system_config.h
CONFIG_FLAGS=config_flags.h

ALL_SOURCE_FILES=( "${ASN1_TYPES_H_FILES} ${ASN1_TYPES_C_FILES} ${H_FILES} ${C_FILES} ${CC_FILES} ${ADS_FILES} \
${ADB_FILES} ${DRIVER_INTERFACE_FILES} ${THREAD_FILES} ${INTERFACE_FILES} ${TRANSPORT_FILES} ${ROUTING_FILES} \
${PACKETIZER_ADAPTER_FILES} ${REQUEST_SIZE_FILE} ${SYSTEM_CONFIG_FILE} ${CONFIG_FLAGS}" )

for SOURCE_FILE_NAME in $ALL_SOURCE_FILES; do
    cp ${SOURCE_FILE_NAME} $CD_INSTALLATION_DIRECTORY/rs485_seds/
done

CD_DATAVIEW_DIR=${CD_INSTALLATION_DIRECTORY}/rs485_seds/dataview
ASN1_CONFIG_XML=${CD_DATAVIEW_DIR}/rs485_sedsConfiguration.xml

mkdir ${CD_DATAVIEW_DIR}
asn1scc --xml-ast ${ASN1_CONFIG_XML} rs485_sedsconfiguration.asn
ASN1_CONFIG_DATA_TYPES=$(xmllint --xpath "//ExportedType/@Name" ${ASN1_CONFIG_XML} | sed -e "s/Name=//g" | sed -e "s/\"//g" | sed -e "s/-/_/g")
cat ${ASN1_FILES_WITHOUT_CONFIG} ${TOOL_INST}/share/taste-types/taste-types.asn work/system.asn > ${CD_DATAVIEW_DIR}/dataview-uniq.asn
sed -i "s/END/END\n/g" ${CD_DATAVIEW_DIR}/dataview-uniq.asn
asn1scc -o ${CD_DATAVIEW_DIR} -typePrefix asn1Scc -renamePolicy 3 -equal -fp AUTO -c -ACN ${CD_DATAVIEW_DIR}/dataview-uniq.asn
cp ${CD_DATAVIEW_DIR}/dataview-uniq.h $CD_INSTALLATION_DIRECTORY/rs485_seds
cp ${CD_DATAVIEW_DIR}/dataview-uniq.c $CD_INSTALLATION_DIRECTORY/rs485_seds
rm -r ${CD_DATAVIEW_DIR}

for ASN1_CONFIG_DATA_TYPE in $ASN1_CONFIG_DATA_TYPES; do
    find $CD_INSTALLATION_DIRECTORY/rs485_seds -type f | xargs sed -i "s/asn1Scc${ASN1_CONFIG_DATA_TYPE}/${ASN1_CONFIG_DATA_TYPE}/g"
done
find $CD_INSTALLATION_DIRECTORY/rs485_seds -type f | xargs sed -i "s/asn1Scc/${PARTITION_NAME}_asn1Scc/gi"
find $CD_INSTALLATION_DIRECTORY/rs485_seds -type f | xargs sed -i "s/request_size.h/${PARTITION_NAME}_request_size.h/g"
find $CD_INSTALLATION_DIRECTORY/rs485_seds -type f | xargs sed -i "s/routing.h/${PARTITION_NAME}_routing.h/g"
sed -i "s/GENERIC_PARTITION_BUFFER_SIZE/${PARTITION_NAME}_GENERIC_PARTITION_BUFFER_SIZE/g" $CD_INSTALLATION_DIRECTORY/rs485_seds/request_size.h
sed -i "s/..\/..\/system_config.h/system_config.h/g" $CD_INSTALLATION_DIRECTORY/rs485_seds/C_ASN1_Types.h
sed -i "s/${PARTITION_NAME}_asn1SccUint/asn1SccUint/g" $CD_INSTALLATION_DIRECTORY/rs485_seds/dataview-uniq.h
sed -i "s/${PARTITION_NAME}_asn1SccUint/asn1SccUint/g" $CD_INSTALLATION_DIRECTORY/rs485_seds/dataview-uniq.c
sed -i "s/${PARTITION_NAME}_asn1SccSint/asn1SccSint/g" $CD_INSTALLATION_DIRECTORY/rs485_seds/dataview-uniq.h
sed -i "s/${PARTITION_NAME}_asn1SccSint/asn1SccSint/g" $CD_INSTALLATION_DIRECTORY/rs485_seds/dataview-uniq.c
sed -i "/^} ${PARTITION_NAME}_asn1Sccrs485_seds_Private_Data;.*/a typedef ${PARTITION_NAME}_asn1Sccrs485_seds_Private_Data rs485_seds_private_data;" $CD_INSTALLATION_DIRECTORY/rs485_seds/dataview-uniq.h
sed -i "s/GENERATED_ASN1SCC/GENERATED_${PARTITION_NAME}_ASN1SCC/g" $CD_INSTALLATION_DIRECTORY/rs485_seds/dataview-uniq.h
sed -i '/^#include "asn1crt.h".*/a #include <drivers_config.h>' $CD_INSTALLATION_DIRECTORY/rs485_seds/dataview-uniq.h
sed -i "s/put(sender_pid,/put((asn1SccPID)0,/g" $CD_INSTALLATION_DIRECTORY/rs485_seds/driver_transport.c

for name in $FUNCTION_NAMES; do
    FUNCTION_PID=$(echo ${name,,} | sed -e "s/name=/PID_/g" | sed -e "s/\"//g")
    find $CD_INSTALLATION_DIRECTORY/rs485_seds -type f | xargs sed -i "s/${FUNCTION_PID}/${PARTITION_NAME}_${FUNCTION_PID}/g"
done

find $CD_INSTALLATION_DIRECTORY/rs485_seds -type f | xargs sed -i "s/PID_env/${PARTITION_NAME}_PID_env/g"

sed -i "s/ERR_/${PARTITION_NAME}_ERR_/g" $CD_INSTALLATION_DIRECTORY/rs485_seds/dataview-uniq.h
sed -i "s/ERR_/${PARTITION_NAME}_ERR_/g" $CD_INSTALLATION_DIRECTORY/rs485_seds/dataview-uniq.c

pushd $CD_INSTALLATION_DIRECTORY/rs485_seds
mv request_size.h "${PARTITION_NAME}_request_size.h"
mv routing.h "${PARTITION_NAME}_routing.h"
mv routing.c "${PARTITION_NAME}_routing.c"
popd
