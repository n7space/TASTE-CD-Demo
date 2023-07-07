/* Body file for GUI ScienceGUI
 * Generated by TASTE (kazoo/templates/skeletons/gui-body/function.tmplt)
 * DO NOT EDIT THIS FILE MANUALLY - MODIFY THE KAZOO TEMPLATE IF NECESSARY
*/
#include <unistd.h>
#include <mqueue.h>

#include "queue_manager.h"
#include "sciencegui.h"
#include "sciencegui_enums_def.h"

typedef struct _PI_Messages {
   T_sciencegui_PI_list msg_id;
   union {
      asn1SccDataStream_Data datastream_param;
   } msg_data;
} PI_Messages;

typedef struct _RI_Messages {
} RI_Messages;

// Queues of messages going from the binary to the user (PIs, or TMs)
static mqd_t sciencegui_PI_queue_id,
             sciencegui_PI_Python_queue_id;



void sciencegui_startup(void)
{
   unsigned msgsize_max = 8192;
   FILE *f = fopen("/proc/sys/fs/mqueue/msgsize_max", "r");
   fscanf(f, "%d", &msgsize_max);

   if (msgsize_max < sizeof (PI_Messages) || msgsize_max < sizeof (RI_Messages)) {
      printf("[ERROR] The GUI is passing a message which parameter size "
             "exceeds your system limit (which is %d bytes per message).\n"
             "You can extend this limit by running: \n"
             "    echo NUMBER | sudo tee /proc/sys/fs/mqueue/msgsize_max\n"
             " ...  with NUMBER > %ld\n"
             "You can also make it permanent (check TASTE wiki)\n\n",
             msgsize_max,
             sizeof(PI_Messages) > sizeof(RI_Messages) ? sizeof(PI_Messages):sizeof(RI_Messages));
      exit(1);
   }

    char *gui_queue_name = NULL;
    int  len;

    len = snprintf (gui_queue_name, 0, "%d_sciencegui_PI_queue", geteuid());
    gui_queue_name = (char *) malloc ((size_t) len + 1);
    if (NULL != gui_queue_name) {
       snprintf (gui_queue_name, len + 1, "%d_sciencegui_PI_queue", geteuid());

       create_exchange_queue(gui_queue_name, 10, sizeof(PI_Messages), &sciencegui_PI_queue_id);

       free (gui_queue_name);
       gui_queue_name = NULL;
    }

    len = snprintf (gui_queue_name, 0, "%d_sciencegui_PI_Python_queue", geteuid());
    gui_queue_name = (char *) malloc ((size_t) len + 1);
    if (NULL != gui_queue_name) {
       snprintf (gui_queue_name, len + 1, "%d_sciencegui_PI_Python_queue", geteuid());

       /* Extra queue for the TM sent to the Python mappers */
       create_exchange_queue(gui_queue_name, 10, sizeof (PI_Messages), &sciencegui_PI_Python_queue_id);

       free (gui_queue_name);
       gui_queue_name = NULL;
    }
}


void sciencegui_PI_dataStream
      (const asn1SccDataStream_Data *IN_datastream)

{
   write_message_to_queue
      (sciencegui_PI_queue_id,
       sizeof(asn1SccDataStream_Data),
       (void*)IN_datastream,
       i_dataStream);

   write_message_to_queue
      (sciencegui_PI_Python_queue_id,
       sizeof(asn1SccDataStream_Data),
       (void*)IN_datastream,
       i_dataStream);
}

