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
#include <stdbool.h>


// Node ID -> array index
uint8_t _nodeid_index = 0;
uint16_t _nodeid[SENDER_NUM];

// For round-robin-send
uint8_t request_index = 0;

// For priority testing
uint16_t _randseed[SENDER_NUM];
uint8_t _packet[PRIORITY_REQUEST_SIZE];
uint16_t _counter = 0;

/*---------------------------------------------------------------------------*/
// Base station's state machine
enum State { Requesting, Priority, Receiving };

static enum State state = Requesting;
uint16_t init_packets_pending = SENDER_NUM;

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
  /* Handling inital packets that will register a node's identity. */
  if (init_packets_pending > 0) {
    uint16_t nodeid = data[1] * 256 + data[0];
    printf("Received register: %u.\n", nodeid);

    init_packets_pending--;
    getIndex(nodeid);

    return;
  }

  uint16_t nodeid = from->u8[1]*256 + from->u8[0];
  uint8_t index = getIndex(nodeid);

  // Set next state
  state = Priority;

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
/*
 * HOTFIX [26th Feb]
 * Now 1-4 nodes are selected to send priority messages with in each PRIORITY_INTERVAL_SEC interval (changed to 10 seconds in project-conf.h).
 */

void getRandSeed(){
  int i;
  for(i = 0;i<_nodeid_index;i++){
    _randseed[i] = 0;
  }
  for(i = 0 ; i < MAX_PRIORITY_NUMBER ; i++){
    int node_id = random_rand()%_nodeid_index;
    _randseed[node_id] = random_rand() % PRIORITY_INTERVAL_SEC;
    while(_randseed[node_id] == 0){
      _randseed[node_id] = random_rand() % PRIORITY_INTERVAL_SEC;
    }
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
void checkPriority(){
  int i;
  for(i = 0 ; i < _nodeid_index ; i++){
    if(_randseed[i] == _counter) {
      printf("Sending priority request to %d.\n", _nodeid[i]);
      sendPriorityRequest(_nodeid[i]);
    }
  }
}

/*---------------------------------------------------------------------------*/

static void
send_request(uint8_t nodeid) {
  _packet[0] = NORMAL_REQUEST_0;
  _packet[1] = NORMAL_REQUEST_1;
  _packet[2] = nodeid % 256;
  _packet[3] = nodeid / 256;

  packetbuf_copyfrom(_packet, sizeof(_packet));
  broadcast_send(&broadcast);
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(base_station_process, ev, data)
{
  static struct etimer et_priority;
  static struct etimer et_second;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  broadcast_open(&broadcast, 129, &broadcast_call);

  /* Waiting to other nodes to join the network.*/
  while (init_packets_pending > 0) {
    PROCESS_PAUSE();
  }

#if ENABLE_PRIORITY_PACKET
  etimer_set(&et_priority, PRIORITY_INTERVAL_SEC * CLOCK_SECOND);
  etimer_set(&et_second, CLOCK_SECOND);
#endif

  while(1) {
    if (state == Priority) {
     // if no priority request enqueued to send
     state = Requesting;
   }

    if(state == Requesting) {
      // Requesting a message in a Round-Robin fashion
      request_index = (request_index + 1) % SENDER_NUM;

      // Transmit to the node
      // int i = 0;
      // for (;i < 9; ++i) {
      //   printf("%u\n", _nodeid[i]);
      // }
      send_request(_nodeid[request_index]);

      // Wait for a receive
      state = Receiving;
      PROCESS_YIELD();
    }

    // printf("State: %d\n", state);

#if ENABLE_PRIORITY_PACKET
    PROCESS_YIELD();
    if(ev == PROCESS_EVENT_TIMER && data == &et_priority){
      getRandSeed();
      etimer_reset(&et_priority);
      _counter = 0;
      printf("Generating random seed.\n");
    } else if(ev == PROCESS_EVENT_TIMER && data == &et_second){
      /*
       * HOTFIX [26th Feb]
       * _counters++ is now executed before checkPriority() to ensure it ignores the zeros in _randseed[].
       * That means at most 4 nodes selecetd in getRandSeed() will be requested for Priority Responses.
       */
      _counter++;
      checkPriority();
      etimer_reset(&et_second);
    }

#endif
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
