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
#include "packet.h"

/*---------------------------------------------------------------------------*/
struct sender_packet_t packets_to_send[MAX_SENDER_QUEUE];
uint8_t queue_size = 0;
/*---------------------------------------------------------------------------*/
PROCESS(sender_mote_process, "Sender motes");
AUTOSTART_PROCESSES(&sender_mote_process);
/*---------------------------------------------------------------------------*/
static struct broadcast_conn broadcast;
uint16_t count = 1;
struct sender_packet_t packet;
/*---------------------------------------------------------------------------*/
static void
push_packet(void) {
  memcpy(&packets_to_send[queue_size], &packet, sizeof(packet));
  queue_size++;
}

/* Pop enqueued packet to packet. */
static void
pop_packet(void) {
  memcpy(&packet, &packets_to_send[0], sizeof(packet));
  memcpy(&packets_to_send, &packets_to_send[1], sizeof(packet) * (queue_size - 1));
  queue_size--;
}

static void
generate_nack(struct sender_packet_t* packet) {
  packet->type = SENDER_NACK;
}

static void
getPriorityPacket(struct sender_packet_t* packet){
  packet->packet = PRIORITY_RESPONSE;

  packet->type = SENDER_ACK;
}

static void
broadcast_control_message(){
  packetbuf_copyfrom(&packet, sizeof(packet));
  broadcast_send(&broadcast);
#if DEBUG_ENABLED
  printf("NACK %u\n", packet.type);
#endif
}
/*---------------------------------------------------------------------------*/

void broadcast_message(){
  packetbuf_copyfrom(&packet, sizeof(packet));
  broadcast_send(&broadcast);
#if DEBUG_ENABLED
  printf("broadcast message sent: %d\n", count);
#endif
}

static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  struct base_packet_t *data = (struct base_packet_t *)packetbuf_dataptr();
  uint16_t nodeid = linkaddr_node_addr.u8[1]*256 + linkaddr_node_addr.u8[0];

  /* Receive base request */
  if(data->request_type == BASE_REQUEST && data->nodeid == nodeid) {
    printf("Received base request with for node_id %u\n", nodeid);

    if (queue_size > 0) {
      pop_packet();
      broadcast_message();
    } else {
      // If no response is on, generate a nack
      generate_nack(&packet);
      broadcast_control_message();
    }
  }

#if ENABLE_PRIORITY_PACKET
  // When overhear a Priority request, check if this is for itself.
  if(data->request_type == PRIORITY_REQUEST && data->nodeid == nodeid){
#if DEBUG_ENABLED
    printf("Received priority request with data packet %u for node_id %u\n", datapacket[1], nodeid);
#endif
    getPriorityPacket(&packet);
    broadcast_message();
  }
#endif
}

static const struct broadcast_callbacks broadcast_call = {broadcast_recv};

// Generates a packet with the counter value.
void getNextPacket(struct sender_packet_t* packet){
  packet->packet = count;
  count++;
  if(count == PRIORITY_RESPONSE)
    count = 1;

  packet->type = SENDER_ACK;
}
/*---------------------------------------------------------------------------*/

static void
send_node_id(void *ptr) {
  /* Send some useless data to join the star topology. */
  packetbuf_copyfrom(&packet, sizeof(packet));
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
      getNextPacket(&packet);
      push_packet();

      etimer_reset(&et_periodic);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
