/**
 * \file
 *         Course work 2 base station code.
 * \author
 *         AESE research group
 */

#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"
#include "project-conf.h"
#include <stdio.h>


// Node ID -> array index
uint8_t _nodeid_index = 0;
uint16_t _nodeid[SENDER_NUM];

// For priority testing
uint16_t _randseed[SENDER_NUM];
uint8_t _packet[PRIORITY_REQUEST_SIZE];
uint16_t _counter = 0;

/*---------------------------------------------------------------------------*/
uint8_t node_count = SENDER_NUM;

/*---------------------------------------------------------------------------*/
PROCESS(base_station_process, "Broadcast example");
AUTOSTART_PROCESSES(&base_station_process);
/*---------------------------------------------------------------------------*/

uint8_t getIndex(uint16_t nodeid){
  int i;
  for(i = 0 ; i < _nodeid_index ; i++){
    if(_nodeid[i] == nodeid)
      return i;
  }
  _nodeid[_nodeid_index] = nodeid;
  _nodeid_index++;
  return (_nodeid_index - 1);
}

// Base station receiver for priority and periodic messages.
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  uint16_t *data = (uint16_t *)packetbuf_dataptr();
  uint16_t nodeid = from->u8[1]*256 + from->u8[0];
  uint8_t index = getIndex(nodeid);

  /* Handling node joins. */
  if (node_count > 0) {
    printf("Node join: %u\n", nodeid);
    node_count -= 1;
    return;
  }

  // When receiving a priority response.
  if(data[0] == PRIORITY_RESPONSE){
    printf("[Priority] received from %d.%d: '%d'\n",
           from->u8[0], from->u8[1], *data);
    return;
  }

    printf("[Periodic] received from %d.%d: '%u'\n",
           from->u8[0], from->u8[1], data[0]);
}

static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;

// Initialize the node ids to 0.
void init(){
  int i;
  for(i = 0 ; i < 9 ; i++){
    _nodeid[i] = 0;
  }
}

// Generates random seed for all seen nodes between <0 - 30> seconds
void getRandSeed(){
  int i;
  for(i = 0 ; i < _nodeid_index ; i++){
    _randseed[i] = random_rand() % PRIORITY_INTERVAL_SEC;
  }
}

// Set the first 2 bytes to 0 for priority messages and the last 2 bytes to node id
void getPriorityRequestPacket(uint16_t nodeid){
  _packet[0] = 0;
  _packet[1] = 0;
  _packet[2] = nodeid % 256;
  _packet[3] = nodeid / 256;
}

// Sends a priority request to a sender node with the corresponding node id
void sendPriorityRequest(uint16_t nodeid) {
  getPriorityRequestPacket(nodeid);
  packetbuf_copyfrom(_packet, sizeof(_packet));
  broadcast_send(&broadcast);
}

// Checks whether a priority request is already enqueued
void checkPriorty(){
  int i;
  for(i = 0 ; i < _nodeid_index ; i++){
    if(_randseed[i] == _counter) {
      printf("Sending priority request to %d.\n", _nodeid[i]);
      sendPriorityRequest(_nodeid[i]);
    }
  }
}


/*---------------------------------------------------------------------------*/
PROCESS_THREAD(base_station_process, ev, data)
{
  static struct etimer et_priority;
  static struct etimer et_second;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  broadcast_open(&broadcast, 129, &broadcast_call);

  /* Waiting for sender to join. */
  while (node_count > 0) {
    PROCESS_PAUSE();
  }

#if ENABLE_PRIORITY_PACKET
  etimer_set(&et_priority, PRIORITY_INTERVAL_SEC * CLOCK_SECOND);
  etimer_set(&et_second, CLOCK_SECOND);
#endif
  while(1) {

#if ENABLE_PRIORITY_PACKET
    PROCESS_YIELD();
    if(ev == PROCESS_EVENT_TIMER && data == &et_priority){
      getRandSeed();
      etimer_reset(&et_priority);
      _counter = 0;
      printf("Generating random seed.\n");
    } else if(ev == PROCESS_EVENT_TIMER && data == &et_second){
      checkPriorty();
      _counter++;
      etimer_reset(&et_second);
    }

#endif

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
