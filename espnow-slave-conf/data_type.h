#include <Arduino.h>

typedef struct __attribute((__packed__)) {
  uint8_t from[6];
  uint8_t to[6];
  uint16_t battery;
  uint32_t temperature;
  uint32_t humidity;
  uint32_t ms;
  uint32_t sum;
} CMMC_SENSOR_T;

