#include <Arduino.h>

typedef struct __attribute((__packed__)) {
  uint16_t battery;
  uint32_t temperature;
  uint32_t humidity;
  uint32_t ms;
} CMMC_SENSOR_T;

