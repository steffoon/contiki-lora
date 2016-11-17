#include "contiki.h"
#include "sys/etimer.h"

#include "lora-radio-arch.h"

#include <stdio.h> /* For printf() */
/*---------------------------------------------------------------------------*/
PROCESS(radiotest_process, "Radio test process");
AUTOSTART_PROCESSES(&radiotest_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(radiotest_process, ev, data)
{
   static struct etimer et;
  PROCESS_BEGIN();

   /* Delay 1 second */
   etimer_set(&et, CLOCK_SECOND*2);
   static  int len = 0;
   static uint8_t MYbuffer[255];
   for(len=0; len<255; len++)
	   MYbuffer[len] = len;
   len = 1;
   while(1)
   {
     PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
     /* Reset the etimer to trig again in 1 second */
     etimer_reset(&et);
     len = (len%255)+1;
     lora_radio_driver.send(MYbuffer, len);
     printf("Sent a packet with %d bytes\n", len);
   }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
