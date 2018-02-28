#ifndef _PACKET_H_
#define _PACKET_H_

struct __attribute__((packed, aligned(1))) packet_t {
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

#endif
