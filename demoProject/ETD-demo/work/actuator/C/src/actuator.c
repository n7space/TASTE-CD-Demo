/* Body file for function Actuator
 * Generated by TASTE (kazoo/templates/skeletons/c-body/function.tmplt)
 * You can edit this file, it will not be overwritten

    !! IMPORTANT                                                        !!
    !! When you modify your design (interface view), you must update    !!
    !! the procedures corresponding to the provided interfaces in this  !!
    !! file. The up-to-date signatures can be found in the header file. !!
*/
#include "actuator.h"
#include <stdio.h>

#define TARGET_HEIGHT 0.0
#define MAX_ACCELERATION 2.0

static asn1SccDeviceState actuatorState;
static asn1Real acceleration;

void actuator_startup(void)
{
   actuatorState = DeviceState_off;
}

void actuator_PI_actuatorControl
      (const asn1SccActuatorControl_Data *IN_actuatorcontroldata)

{
   actuatorState = IN_actuatorcontroldata->state_request;
}


void actuator_PI_tick(void)
{
   if(actuatorState == DeviceState_on)
   {
       asn1SccSystemDataRequest_Data systemDataRequest;
       actuator_RI_SystemDataRequest(&systemDataRequest);

       asn1Real heightDifference = TARGET_HEIGHT - systemDataRequest.system_phisic_attrs.height;

       acceleration = heightDifference * heightDifference * heightDifference / 10000.0;

       if(heightDifference > 0.0)
       {
         heightDifference = heightDifference - 4.0;
         acceleration = heightDifference * heightDifference * heightDifference / 10000.0;
       }
       else
       {
         heightDifference = heightDifference + 4.0;
         acceleration = heightDifference * heightDifference * heightDifference / 10000.0;
       }

       if(acceleration > MAX_ACCELERATION)
       {
           acceleration = MAX_ACCELERATION;
       }
       if(acceleration < -MAX_ACCELERATION)
       {
           acceleration = -MAX_ACCELERATION;
       }

       asn1SccActuatorStatusUpdate_Data actuatorStatusUpdate;
       actuatorStatusUpdate.device_state = actuatorState;
       actuatorStatusUpdate.acceleration = acceleration;
       actuator_RI_ActuatorStatusUpdate(&actuatorStatusUpdate);

       asn1SccPhisicValType actuation = acceleration;
       actuator_RI_actuate(&actuation);
   }
   else
   {
       asn1SccPhisicValType actuation = 0.0;
       actuator_RI_actuate(&actuation);
   }
}