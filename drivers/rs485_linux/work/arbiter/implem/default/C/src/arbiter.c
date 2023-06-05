/* Body file for function Arbiter
 * Generated by TASTE (kazoo/templates/skeletons/c-body/function.tmplt)
 * You can edit this file, it will not be overwritten

    !! IMPORTANT                                                        !!
    !! When you modify your design (interface view), you must update    !!
    !! the procedures corresponding to the provided interfaces in this  !!
    !! file. The up-to-date signatures can be found in the header file. !!
*/
#include "arbiter.h"
//#include <stdio.h>

static uint8_t message[] = {0x18, 0xbb, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xaa, 0xc9, 0xf3};

static asn1SccPlatformSpecificPointer private_data;

void arbiter_startup(void)
{
   // Write your initialisation code
   // You may call sporadic required interfaces and start timers
   // puts ("[Arbiter] Startup");
}

void arbiter_PI_Init
      (const asn1SccInitRequestData *IN_initreqseq)
{
   private_data = IN_initreqseq->private_data;
}

void arbiter_PI_Deliver
      (const asn1SccDeliveredRequestData *IN_deliverreqseq)
{
   arbiter_RI_Deliver(IN_deliverreqseq);
}

void arbiter_PI_Receive
      (const asn1SccReceivedRequestData *IN_receivedreqseq)
{
   arbiter_RI_Receive(IN_receivedreqseq);
}

void arbiter_PI_Doorman( void )
{
   rs485_linux_asn1SccDeliveredRequestData data;
   data.private_data = private_data;
   data.message_data = &message;
   data.length = 13;
   arbiter_PI_Deliver(&data);
}