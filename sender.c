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
PROCESS(sender_mote_process, "Sender motes");
AUTOSTART_PROCESSES(&sender_mote_process);
/*---------------------------------------------------------------------------*/
static struct broadcast_conn broadcast;
uint16_t count = 1;
uint8_t packet[PACKAGE_SIZE];
static uint16_t nodeid;

uint8_t packets_pending = 0;
uint8_t packet_queue[MAX_PENDING_REQUESTS][PACKAGE_SIZE];

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

if(datapacket[0] == NORMAL_REQUEST && datapacket[1] == nodeid) {
    printf("%s\n", "Sending.");
    memcpy(packet, packet_queue[packets_pending], sizeof(packet));
    memcpy(packet_queue, packet_queue + sizeof(packet), sizeof(packet) * (packets_pending - 1));
    packets_pending--;

    broadcast_message(packet);
  }
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

static void
send_register(void *ptr){
  printf("Sending register: %u\n", nodeid);

  packetbuf_copyfrom(&nodeid, sizeof(nodeid));
  broadcast_send(&broadcast);
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sender_mote_process, ev, data)
{
  static struct etimer et_periodic;
  static struct ctimer ct_periodic;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();
  nodeid = linkaddr_node_addr.u8[1]*256 + linkaddr_node_addr.u8[0];

  broadcast_open(&broadcast, 129, &broadcast_call);

  /* Initial send to establish connections. */
  random_init(0);
  uint16_t wait_time = random_rand() % (CLOCK_SECOND * 40);
  ctimer_set(&ct_periodic, INIT_TIME * CLOCK_SECOND + wait_time, send_register, NULL);

  /* Set Timer*/
  etimer_set(&et_periodic, CLOCK_SECOND/5);

  while(1) {
    PROCESS_YIELD();
    if(ev == PROCESS_EVENT_TIMER && data == &et_periodic){
      getNextPacket(packet);

      // Enqueue pending packets to be sent to the source
      memcpy(packet_queue[packets_pending], packet, sizeof(packet));
      packets_pending++;

      etimer_reset(&et_periodic);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
