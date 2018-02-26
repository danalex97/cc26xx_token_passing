/**
 * \file
 *         Course work 2 sender code.
 * \author
 *         AESE research group
 */

#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"
#include "project-conf.h"
#include <stdio.h>

/*---------------------------------------------------------------------------*/
uint8_t packets_to_send[MAX_SENDER_QUEUE][PACKAGE_SIZE];
uint8_t queue_size = 0;
/*---------------------------------------------------------------------------*/
PROCESS(sender_mote_process, "Sender motes");
AUTOSTART_PROCESSES(&sender_mote_process);
/*---------------------------------------------------------------------------*/
static struct broadcast_conn broadcast;
uint16_t count = 1;
uint8_t packet[PACKAGE_SIZE];
/*---------------------------------------------------------------------------*/
static void
push_packet(void) {
  memcpy(packets_to_send[queue_size], packet, sizeof(packet));
  queue_size++;
}

/* Pop enqueued packet to packet. */
static void
pop_packet(void) {
  memcpy(packet, packets_to_send[0], sizeof(packet));
  memcpy(packets_to_send, packets_to_send + sizeof(packet), sizeof(packet) * (queue_size - 1));
  queue_size--;
}
/*---------------------------------------------------------------------------*/
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
  uint16_t *datapacket = (uint16_t *)packetbuf_dataptr();
  uint16_t nodeid = linkaddr_node_addr.u8[1]*256 + linkaddr_node_addr.u8[0];

  /* Receive base request */
  if(datapacket[0] == BASE_REQUEST && datapacket[1] == nodeid) {
    printf("Received base request with for node_id %u\n", nodeid);

    if (queue_size > 0) {
      pop_packet();
      broadcast_message(packet);
    }
  }

#if ENABLE_PRIORITY_PACKET
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

static void
send_node_id(void *ptr) {
  /* Send some useless data to join the star topology. */
  packetbuf_copyfrom(packet, sizeof(packet));
  broadcast_send(&broadcast);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sender_mote_process, ev, data)
{
  static struct etimer et_periodic;
  static struct ctimer ct_init;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();
  broadcast_open(&broadcast, 129, &broadcast_call);

  /* Send node id to base station */
  uint16_t random_timeout = random_rand() % (CLOCK_SECOND * 30);
  uint16_t inital_timeout = CLOCK_SECOND * 10;
  ctimer_set(&ct_init, inital_timeout + random_timeout, send_node_id, NULL);

  /* Wait for base station to function. */
  etimer_set(&et_periodic, CLOCK_SECOND * 40);
  while(1) {
    PROCESS_YIELD();
    if(ev == PROCESS_EVENT_TIMER && data == &et_periodic){
      break;
    }
  }

  /* Set Timer*/
  etimer_set(&et_periodic, CLOCK_SECOND/5);

  while(1) {
    PROCESS_YIELD();
    if(ev == PROCESS_EVENT_TIMER && data == &et_periodic){
      getNextPacket(packet);
      push_packet();

      etimer_reset(&et_periodic);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
