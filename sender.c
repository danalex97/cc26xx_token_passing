/**
 * \file
 *         Course work 2 sender code.
 * \author
 *         AESE research group
 */

#include "contiki.h"
#include "net/rime/rime.h"
#include "net/rime/timesynch.h"
#include "random.h"
#include "project-conf.h"
#include <stdio.h>

/*---------------------------------------------------------------------------*/
PROCESS(sender_mote_process, "Sender motes");
AUTOSTART_PROCESSES(&sender_mote_process);
/*---------------------------------------------------------------------------*/
static struct broadcast_conn broadcast;
uint16_t count = 1;
uint8_t packet[PACKAGE_SIZE];

void getPriorityPacket(uint8_t* packet){
  packet[1] = 255;
  packet[0] = 255;
}

void broadcast_message(uint8_t* packet){
  packetbuf_copyfrom(packet, sizeof(packet));
  broadcast_send(&broadcast);
#if DEBUG_ENABLED
  printf("broadcast message sent: %d\n", count);
#endif
}

static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
#if ENABLE_PRIORITY_PACKET
  uint8_t *datapacket = (uint8_t *)packetbuf_dataptr();
  uint16_t nodeid = linkaddr_node_addr.u8[1]*256 + linkaddr_node_addr.u8[0];

  // When overhear a Priority request, check if this is for itself.
  if(datapacket[0] == PRIORITY_REQUEST && datapacket[1] == nodeid){
#if DEBUG_ENABLED
    printf("Received priority request with data packet %u for node_id %u\n", datapacket[1], nodeid);
#endif
    getPriorityPacket(packet);
    broadcast_message(packet);
  }
#endif
}

static const struct broadcast_callbacks broadcast_call = {broadcast_recv};

// Generates a packet with the counter value.
void getNextPacket(uint8_t* packet){
  packet[1] = count / 256;
  packet[0] = count % 256;
  count++;
  if(count == PRIORITY_RESPONSE)
    count = 1;
}

/*---------------------------------------------------------------------------*/
// Timer for random message wait.
static struct rtimer message_wait;

static void
send_broadcast(struct rtimer *timer, void *ptr){
  getNextPacket(packet);
  broadcast_message(packet);
}

//  rtimer_set(&message_wait, RTIMER_NOW() + RTIMER_SECOND / wait_time, 0, send_sync,         NULL);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sender_mote_process, ev, data)
{
  static struct etimer et_periodic;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  #if TIMESYNCH_CONF_ENABLED
    timesynch_init();
    timesynch_set_authority_level(2);
  #endif
  broadcast_open(&broadcast, 129, &broadcast_call);

  /* Set Timer */
  etimer_set(&et_periodic, CLOCK_SECOND/5);

  while(1) {
    PROCESS_YIELD();
    if(ev == PROCESS_EVENT_TIMER && data == &et_periodic){
      #if TIMESYNCH_CONF_ENABLED
        rtimer_clock_t global_time = timesynch_time();
        rtimer_clock_t local_time  = timesynch_time_to_rtimer(global_time);
      #else
        rtimer_clock_t local_time = RTIMER_NOW();
      #endif

      // broadcast after random timeout
      uint16_t node_id = linkaddr_node_addr.u8[1]*256 + linkaddr_node_addr.u8[0];
      uint16_t slot_nbr = node_id - 1;
      uint16_t slots    = SENDER_NUM;
      rtimer_set(&message_wait, local_time + RTIMER_SECOND / slots * slot_nbr, 0, send_broadcast, NULL);

      // Wait for next event
      etimer_reset(&et_periodic);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
