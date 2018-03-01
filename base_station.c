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
#include "packet.h"
#include "queue.h"
#include <stdbool.h>

// Node ID -> array index
uint8_t _nodeid_index = 0;
uint16_t _nodeid[SENDER_NUM];

// For priority testing
uint16_t _randseed[SENDER_NUM];
struct base_packet_t _packet;
uint16_t _counter = 0;

/*---------------------------------------------------------------------------*/
uint8_t node_count = SENDER_NUM;

enum state_t {
  Requesting,
  Priority,
  Receiving
};

enum state_t state = Requesting; // initial state?

// Current index of the node in the Round-Robin token passing
uint8_t current_index = 0;
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
  struct sender_packet_t *data = (struct sender_packet_t *)packetbuf_dataptr();
  uint16_t nodeid = from->u8[1]*256 + from->u8[0];
  getIndex(nodeid);

  /* Handling node joins. */
  if (node_count > 0) {
    printf("Node join: %u\n", nodeid);
    node_count--;
    return;
  }

  if (data->type == SENDER_ACK) {
    // When receiving a priority response.
    if(data->packet == PRIORITY_RESPONSE){
      printf("[Priority] received from %d.%d.'\n",
             from->u8[0], from->u8[1], data->packet);

      // Notify main process
      state = Priority;
      process_post(&base_station_process, PROCESS_EVENT_CONTINUE, NULL);
      return;
    }

    printf("[Periodic] received from %d.%d: '%u'\n",
           from->u8[0], from->u8[1], data->packet);
  }

  if (data->type == SENDER_NACK) {
    printf("Received nack from %d.%d.\n",
      from->u8[0], from->u8[1]);
  }

  // Notify main process
  state = Priority;
  process_post(&base_station_process, PROCESS_EVENT_CONTINUE, NULL);
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
  _packet.request_type = PRIORITY_REQUEST;
  _packet.nodeid = nodeid;
}

// Sends a priority request to a sender node with the corresponding node id
void addPriorityRequest(uint16_t nodeid) {
  getPriorityRequestPacket(nodeid);

  /* Enque the priority request*/
  push_packet(&_packet);
  // printf("Pushing priority request to %u.\n", _packet.nodeid);
}

// Sends a priority request to a sender node with the corresponding node id
void sendPriorityRequest() {
  pop_packet(&_packet);

  printf("Sending priority request to %u.\n", _packet.nodeid);
  packetbuf_copyfrom(&_packet, sizeof(_packet));
  broadcast_send(&broadcast);
}


// Checks whether a priority request is already enqueued
void checkPriority(){
  int i;
  for(i = 0 ; i < _nodeid_index ; i++){
    if(_randseed[i] == _counter) {
      addPriorityRequest(_nodeid[i]);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
send_request(uint16_t nodeid) {
  _packet.request_type = BASE_REQUEST;
  _packet.nodeid = nodeid;

  printf("Sending base request to: %u\n", nodeid);
  packetbuf_copyfrom(&_packet, sizeof(_packet));
  broadcast_send(&broadcast);
}

/*---------------------------------------------------------------------------*/
#if ENABLE_PRIORITY_PACKET
static struct ctimer ct_priority;
static struct ctimer ct_second;

void
random_seed_handler(void *ptr) {
  getRandSeed();
  _counter = 0;
  printf("Generating random seed.\n");

  ctimer_reset(&ct_priority);
}

void
priority_gen_handler(void *ptr) {
  checkPriority();
  _counter++;

  ctimer_reset(&ct_second);
}

#endif
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(base_station_process, ev, data)
{
  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  broadcast_open(&broadcast, 129, &broadcast_call);
  init_queue(MAX_BASE_QUEUE, sizeof(struct base_packet_t));

  /* Waiting for sender to join. */
  while (node_count > 0) {
    PROCESS_PAUSE();
  }
  printf("%s\n", "Nodes registered.");

  // Inital random seed.
  getRandSeed();

#if ENABLE_PRIORITY_PACKET
  ctimer_set(&ct_second, CLOCK_SECOND,
    random_seed_handler, NULL);
  ctimer_set(&ct_priority, PRIORITY_INTERVAL_SEC * CLOCK_SECOND,
    priority_gen_handler, NULL);
#endif

  while(1) {
    if (state == Priority) {
      if (queue_size() == 0) {
        // If no priority request is pending, send a normal request

        state = Requesting;
      }
#if ENABLE_PRIORITY_PACKET
      else {
        // Send a priority request
        sendPriorityRequest();

        state = Receiving;

        // Yield the process so we can receive packets
        PROCESS_YIELD();
      }
#endif
    }

    if (state == Requesting) {
      // Send request
      send_request(_nodeid[current_index]);

      current_index = (current_index + 1) % SENDER_NUM;

      state = Receiving;

      // Yield the process so we can receive packets
      PROCESS_YIELD();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
