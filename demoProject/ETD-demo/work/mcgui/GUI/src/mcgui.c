/* Body file for GUI MCGUI
 * Generated by TASTE (kazoo/templates/skeletons/gui-body/function.tmplt)
 * DO NOT EDIT THIS FILE MANUALLY - MODIFY THE KAZOO TEMPLATE IF NECESSARY
*/
#include <unistd.h>
#include <mqueue.h>

#include "queue_manager.h"
#include "mcgui.h"
#include "mcgui_enums_def.h"

typedef struct _PI_Messages {
   T_mcgui_PI_list msg_id;
   union {
      asn1SccTM_Data tm_param;
   } msg_data;
} PI_Messages;

typedef struct _RI_Messages {
   T_mcgui_RI_list msg_id;
   union {
      asn1SccTC_Data tc_param;
   } msg_data;
} RI_Messages;

// Queues of messages going from the binary to the user (PIs, or TMs)
static mqd_t mcgui_PI_queue_id,
             mcgui_PI_Python_queue_id;

// Queues of messages going from the user to the binary GUI (RIs, or TCs)
static mqd_t mcgui_RI_queue_id;


void mcgui_startup(void)
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

    len = snprintf (gui_queue_name, 0, "%d_mcgui_RI_queue", geteuid());
    gui_queue_name = (char *) malloc ((size_t) len + 1);
    if (NULL != gui_queue_name) {
       snprintf (gui_queue_name, len + 1, "%d_mcgui_RI_queue", geteuid());

       create_exchange_queue(gui_queue_name, 5, sizeof(RI_Messages), &mcgui_RI_queue_id);

       free (gui_queue_name);
       gui_queue_name = NULL;
    }

    len = snprintf (gui_queue_name, 0, "%d_mcgui_PI_queue", geteuid());
    gui_queue_name = (char *) malloc ((size_t) len + 1);
    if (NULL != gui_queue_name) {
       snprintf (gui_queue_name, len + 1, "%d_mcgui_PI_queue", geteuid());

       create_exchange_queue(gui_queue_name, 10, sizeof(PI_Messages), &mcgui_PI_queue_id);

       free (gui_queue_name);
       gui_queue_name = NULL;
    }

    len = snprintf (gui_queue_name, 0, "%d_mcgui_PI_Python_queue", geteuid());
    gui_queue_name = (char *) malloc ((size_t) len + 1);
    if (NULL != gui_queue_name) {
       snprintf (gui_queue_name, len + 1, "%d_mcgui_PI_Python_queue", geteuid());

       /* Extra queue for the TM sent to the Python mappers */
       create_exchange_queue(gui_queue_name, 10, sizeof (PI_Messages), &mcgui_PI_Python_queue_id);

       free (gui_queue_name);
       gui_queue_name = NULL;
    }
}

//  Function polling the message queue for incoming message from the user
void mcgui_PI_Poll(void)
{
   struct mq_attr msgq_attr;
   char* msgcontent = NULL;

   T_mcgui_RI_list message_received_type;

   if ((msgcontent = (char*)malloc(sizeof(RI_Messages))) == NULL) {
      perror("Error when allocating memory in GUI polling function");
      exit (-1);
   }

   mq_getattr(mcgui_RI_queue_id, &msgq_attr);

   while (!retrieve_message_from_queue(mcgui_RI_queue_id,
                                       sizeof(RI_Messages),
                                       msgcontent,
                                       (int *)&message_received_type)) {
      switch(message_received_type) {
         case i_tc:
            mcgui_RI_tc((asn1SccTC_Data *)msgcontent);
            break;
       default : break;
      }
   }

   free(msgcontent);
   return;
}

void mcgui_PI_tm
      (const asn1SccTM_Data *IN_tmdata)

{
   write_message_to_queue
      (mcgui_PI_queue_id,
       sizeof(asn1SccTM_Data),
       (void*)IN_tmdata,
       i_tm);

   write_message_to_queue
      (mcgui_PI_Python_queue_id,
       sizeof(asn1SccTM_Data),
       (void*)IN_tmdata,
       i_tm);
}


