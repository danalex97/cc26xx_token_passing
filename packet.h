#ifndef _PACKET_H_
#define _PACKET_H_

struct __attribute__((packed, aligned(1))) sender_packet_t {
  /* Same packet identification as provided. */
  uint16_t packet;

  /* Additional data. */
  uint16_t type;
};

struct __attribute__((packed, aligned(1))) base_packet_t {
  /* Same packet identification as provided. */
  uint16_t request_type;
  uint16_t nodeid;

  /* Additional data. */
};

#endif
