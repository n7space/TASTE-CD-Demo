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


void arbiter_startup(void)
{
   // Write your initialisation code
   // You may call sporadic required interfaces and start timers
   // puts ("[Arbiter] Startup");
}

void arbiter_PI_Deliver
      (const asn1SccDeliveredRequestData *IN_deliverreqseq)

{
   // Write your code here
}


void arbiter_PI_Receive
      (const asn1SccReceivedRequestData *IN_receivereqseq)

{
   // Write your code here
}


