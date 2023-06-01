/* Body file for function Arbiter
 * Generated by TASTE (kazoo/templates/skeletons/c-body/function.tmplt)
 * You can edit this file, it will not be overwritten

    !! IMPORTANT                                                        !!
    !! When you modify your design (interface view), you must update    !!
    !! the procedures corresponding to the provided interfaces in this  !!
    !! file. The up-to-date signatures can be found in the header file. !!
*/
#include "arbiter.h"

#include <assert.h>

#define OUTPUT_BUFFER_SIZE 1000

__attribute__((section(".sdramMemorySection")))
static asn1SccPioHwas pioHwas;

void arbiter_startup(void)
{
   asn1SccPioHwasPinConfig pinConfig;
   pinConfig.mPortConfig = asn1SccPioHwas_Port_pioHwas_Port_A;
   pinConfig.mPinConfig = 2;
   pinConfig.mDirectionConfig = asn1SccPioHwas_Direction_pioHwas_Direction_Output;
   pinConfig.mControlConfig = asn1SccPioHwas_Control_pioHwas_Control_Pio;
   arbiter_RI_PioHwas_InitPin_Pi(&pioHwas, &pinConfig);
}

bool arbiter_is_control_packet_received
      (const uint8_t *message_data, size_t length)
{
   if (length == 13 && message_data[8] == 0xFF) {
      return true;
   }
   return false;
}

void arbiter_PI_Deliver
      (const asn1SccDeliveredRequestData *IN_deliverreqseq)
{
   arbiter_RI_Deliver(IN_deliverreqseq);
}

void arbiter_PI_Receive
      (const asn1SccReceivedRequestData *IN_receivereqseq)
{
   if (arbiter_is_control_packet_received(IN_receivereqseq->message_data, IN_receivereqseq->length)) {
      arbiter_RI_PioHwas_SetPin_Pi(&pioHwas);
      arbiter_RI_Control();
   } else {
      arbiter_RI_Receive(IN_receivereqseq);
   }
}

void arbiter_PI_ControlAck
      (void)
{
   arbiter_RI_PioHwas_ResetPin_Pi(&pioHwas);
}
