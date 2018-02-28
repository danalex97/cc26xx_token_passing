#ifndef _PACKET_H_
#define _PACKET_H_

struct __attribute__((packed, aligned(1))) sender_packet_t {
  /* Same packet identification as provided. */
  union {
    uint16_t packet;
    struct {
      uint8_t packet_0;
      uint8_t packet_1;
    };
  };

  /* Additional data. */
  uint8_t type0;
  uint8_t type1;
};

struct __attribute__((packed, aligned(1))) base_packet_t {
  /* Same packet identification as provided. */
  uint16_t request_type;

  union {
    uint16_t nodeid;
    struct {
      uint8_t nodeid_0;
      uint8_t nodeid_1;
    };
  };
};

#endif
