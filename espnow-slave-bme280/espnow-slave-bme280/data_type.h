#include <Arduino.h>

typedef struct __attribute((__packed__)) {
  uint8_t from[6];
  uint8_t to[6];
  uint8_t type = 1;
  uint32_t battery = 0x00;
  uint32_t field1 = 0x00;
  uint32_t field2 = 0x00;
  uint32_t field3 = 0x00;   
  uint32_t field4 = 0x00;
  uint32_t field5 = 0x00;
  uint32_t field6 = 0x00;
  uint8_t nameLen;
  char deviceName[15];
  uint32_t ms = 0;
  uint32_t sum;
} CMMC_SENSOR_T;

typedef struct __attribute((__packed__)) {
  uint8_t header[2] = {0x7e, 0x7f};
  uint8_t version = 1;
  uint32_t reserved = 0xffffffff;
  CMMC_SENSOR_T data;
  uint32_t sleep;
  uint32_t ms;
  uint32_t sum;
  uint8_t tail[2] = {0x0d, 0x0a};
} CMMC_PACKET_T;

