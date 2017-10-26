#include <Arduino.h>

typedef struct __attribute((__packed__)) {
  // TODO: CHANGE TYPE
  uint16_t type = 2;
  uint8_t from[6];
  uint8_t to[6];
  uint32_t battery = 0x00;
  uint32_t temperature;
  uint32_t humidity;
  uint32_t field3 = 0x41;   
  uint32_t field4 = 0x42;
  uint32_t field5 = 0x43;
  uint32_t field6 = 0x44;
  uint32_t ms = 0;
  uint32_t sum;
} CMMC_SENSOR_T;

typedef struct __attribute((__packed__)) {
  uint8_t header[2] = {0x7e, 0x7f};
  uint8_t version = 2;
  uint32_t reserved = 0x00;
  CMMC_SENSOR_T data;
  uint32_t sleep;
  uint32_t ms;
  uint32_t sum;
  uint8_t tail[2] = {0x0d, 0x0a};
} CMMC_PACKET_T;

