#include <Arduino.h>

typedef struct __attribute((__packed__)) {
  uint8_t from[6];
  uint8_t to[6];
  uint16_t battery;
  float temperature;
  float humidity;
  uint32_t ms;
  uint32_t sum;
} CMMC_SENSOR_T;

typedef struct __attribute((__packed__)) {
  uint8_t header[2];
  uint32_t reserved;
  uint16_t type;
  CMMC_SENSOR_T data;
  uint32_t sum;
  uint8_t tail[2];
} CMMC_PACKET_T;

