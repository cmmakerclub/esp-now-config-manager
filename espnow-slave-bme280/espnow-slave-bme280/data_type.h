#include <Arduino.h>

typedef struct __attribute((__packed__)) {
  uint8_t from[6];
  uint8_t to[6];
  uint8_t type = 0;
  uint16_t battery = 0x00;
  uint32_t field1 = 0x00;
  uint32_t field2 = 0x00;
  uint32_t field3 = 0x00;   
  uint32_t field4 = 0x00;
  uint32_t field5 = 0x00;
  uint32_t field6 = 0x00;
  uint8_t nameLen = 0x00;
  char myName[16]; 
  uint32_t ms = 0;
  uint16_t sent_ms = 0;
  uint32_t sum = 0;
} CMMC_SENSOR_T; 