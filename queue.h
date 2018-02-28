#ifndef _QUEUE_H_
#define _QUEUE_H_

#include "contiki.h"
#include "queue.h"
#include <stdio.h>

#define MAX_QUEUE_SIZE 20

static uint8_t _queue_size = 0;
static uint8_t _capacity;
static uint8_t _size;
static uint8_t queue[MAX_QUEUE_SIZE];

void
init_queue(uint8_t capacity, uint8_t size) {
  _capacity = capacity;
  _size = size;
}

void
push_packet(void *packet) {
  if (_queue_size >= _capacity) {
    printf("Queue overflow\n");
  }
  memcpy(&queue[_queue_size * _size], packet, _size);
  _queue_size++;
}

/* Pop enqueued packet to packet. */
void
pop_packet(void *packet) {
  memcpy(packet, &queue[_queue_size - _size], _size);
  _queue_size--;
}

uint8_t
queue_size() {
  return _queue_size;
}

#endif
