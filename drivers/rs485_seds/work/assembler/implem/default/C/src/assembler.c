/* Body file for function Assembler
 * Generated by TASTE (kazoo/templates/skeletons/c-body/function.tmplt)
 * You can edit this file, it will not be overwritten

    !! IMPORTANT                                                        !!
    !! When you modify your design (interface view), you must update    !!
    !! the procedures corresponding to the provided interfaces in this  !!
    !! file. The up-to-date signatures can be found in the .ads file.   !!
*/
#include "assembler.h"

#include <assert.h>

#include <FreeRTOS.h>
#include <semphr.h>

#include <Utils/ByteFifo.h>
#include <Escaper.h>
#include <Broker.h>

#ifndef BROKER_BUFFER_SIZE
#define BROKER_BUFFER_SIZE 256
#endif

__attribute__((section(".sdramMemorySection")))
static asn1SccUartHwas uart;

__attribute__((section(".sdramMemorySection")))
static Escaper escaper;
#define ENCODED_PACKET_BUFFER_SIZE 256
__attribute__((section(".sdramMemorySection")))
static uint8_t encodedPacketBuffer[ENCODED_PACKET_BUFFER_SIZE] = {""};
#define DECODED_PACKET_BUFFER_SIZE BROKER_BUFFER_SIZE
__attribute__((section(".sdramMemorySection")))
static uint8_t decodedPacketBuffer[DECODED_PACKET_BUFFER_SIZE] = {""};

#define OUTPUT_BUFFER_SIZE 65536
__attribute__((section(".sdramMemorySection")))
static uint8_t outputQueueBuffer[OUTPUT_BUFFER_SIZE];
__attribute__((section(".sdramMemorySection")))
static ByteFifo outputQueue;

__attribute__((section(".sdramMemorySection")))
static asn1SccSystemBus busId;

void assembler_startup(void)
{
}

asn1SccUartHwas_Instance assembler_convert_conf_asn_devname_to_hwas
      (asn1SccRS485_SEDS_Device_T devname)
{
   switch (devname) {
   case asn1SccRS485_SEDS_Device_T_uart4:
      return UartHwas_Instance_uartHwas_Instance_4;
   case asn1SccRS485_SEDS_Device_T_uart3:
      return UartHwas_Instance_uartHwas_Instance_3;
   case asn1SccRS485_SEDS_Device_T_uart2:
      return UartHwas_Instance_uartHwas_Instance_2;
   case asn1SccRS485_SEDS_Device_T_uart1:
      return UartHwas_Instance_uartHwas_Instance_1;
   case asn1SccRS485_SEDS_Device_T_uart0:
   default:
      return UartHwas_Instance_uartHwas_Instance_0;
   }
}

asn1SccUartHwas_Instance assembler_convert_conf_asn_baudrate_to_hwas
      (asn1SccRS485_SEDS_Baudrate_T baudrate)
{
   switch (baudrate) {
   case asn1SccRS485_SEDS_Baudrate_T_b115200:
      return UartHwas_Baudrate_uartHwas_Baudrate115200;
   case asn1SccRS485_SEDS_Baudrate_T_b9600:
   default:
      return UartHwas_Baudrate_uartHwas_Baudrate9600;
   }
}

void assembler_invoke_broker_receive_packet
      (enum SystemBus bus_id, uint8_t* const data, const size_t data_size)
{
   asn1SccReceivedRequestData request_data;
   request_data.bus_id = bus_id;
   request_data.message_data = (asn1SccPlatformSpecificPointer) data;
   request_data.length = data_size;
   assembler_RI_Receive(&request_data);
}

void assembler_send_single_byte_to_uarthwas(void)
{
   asn1SccUartHwasInterfaceType_SendByteAsyncCmd_Type sendByteStructure = {
      .uart = uart
   };

   bool isBytePresent = ByteFifo_pull(&outputQueue, (uint8_t *) &sendByteStructure.byteToSend);
   if (isBytePresent) {
      assembler_RI_UartHwas_SendByteAsyncCmd_Pi(&sendByteStructure);
   } else {
      // No more bytes in queue, send acknowledgment to Arbiter.
      assembler_RI_TransmissionEnded();
   }
}

void assembler_PI_Init
      (const asn1SccInitRequestData *IN_initreqseq)

{
   assert(IN_initreqseq);
   Escaper_init(&escaper,
                encodedPacketBuffer,
                ENCODED_PACKET_BUFFER_SIZE,
                decodedPacketBuffer,
                DECODED_PACKET_BUFFER_SIZE);
   ByteFifo_init(&outputQueue, outputQueueBuffer, OUTPUT_BUFFER_SIZE);

   busId = IN_initreqseq->bus_id;

   /* Initialize UART */
   asn1SccUartHwasConfig config;
   asn1SccRS485_SEDS_Conf_T *conf = (asn1SccRS485_SEDS_Conf_T *) IN_initreqseq->device_configuration;
   config.mInstance = assembler_convert_conf_asn_devname_to_hwas(conf->devname);
   config.mBaudrate = assembler_convert_conf_asn_baudrate_to_hwas(conf->speed);
   assembler_RI_UartHwas_InitUartCmd_Pi(&uart, &config);

   /* We need to call Pi interface to enable Read interrupts. Otherwise
   ** we will not get any byte from the interface. */
   asn1SccUartHwasInterfaceType_ReadByteAsyncCmd_Type readByte;
   readByte.uart = uart;
   assembler_RI_UartHwas_ReadByteAsyncCmd_Pi(&readByte);
}

void assembler_PI_EnableTransmission(void)
{
   assembler_send_single_byte_to_uarthwas();
}

void assembler_PI_Deliver
      (const asn1SccDeliveredRequestData *IN_deliverreqseq)

{
   uint64_t length = (uint64_t) IN_deliverreqseq->length;
   uint8_t *data = (uint8_t *) IN_deliverreqseq->message_data;
   size_t index = 0;

   Escaper_start_encoder(&escaper);
   size_t encodedBytes = Escaper_encode_packet(&escaper, data, length, &index);
   for (size_t i = 0; i < encodedBytes; ++i) {
      bool result = ByteFifo_push(&outputQueue, encodedPacketBuffer[i]);
      assert(result && "Assembler output queue exceeded");
   }
}

void assembler_PI_UartHwas_ReadByteAsyncCmd_Ri
      (const asn1SccUartHwasInterfaceType_ReadByteAsyncCmd_TypeNotify *IN_inputparam)

{
   Escaper_decode_packet(&escaper, busId, (uint8_t *) &IN_inputparam->byteToRead, 1, &assembler_invoke_broker_receive_packet);
}

void assembler_PI_UartHwas_SendByteAsyncCmd_Ri
      (const asn1SccUartHwasInterfaceType_SendByteAsyncCmd_TypeNotify *IN_inputparam)

{
   (void)IN_inputparam;
   assembler_send_single_byte_to_uarthwas();
}

void assembler_PI_UartErrorReporting_OverrunError_Ri(void)
{
}
