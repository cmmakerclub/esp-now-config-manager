#include <Arduino.h>

typedef struct __attribute((__packed__)) {
  uint16_t type = 0xffffff;
  uint8_t from[6];
  uint8_t to[6];
  uint32_t battery = 0x00;
  uint32_t temperature;
  uint32_t humidity;
  uint32_t field3 = 0x41;
  uint32_t field4 = 0x42;
  uint32_t field5 = 0x43;
  uint32_t field6 = 0x44;
  uint32_t field7 = 0x45;
  uint32_t field8 = 0x46;
  uint32_t ms = 0;
  uint32_t sum;
} CMMC_SENSOR_T;

