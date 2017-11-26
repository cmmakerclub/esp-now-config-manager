#ifndef CMMC_Packet_H
#define CMMC_Packet_H

#include <Arduino.h>

#include <Arduino.h>

typedef struct __attribute((__packed__)) {
  uint16_t battery;
  uint32_t field1;
  uint32_t field2;
  uint32_t field3;
  uint32_t field4;
} CMMC_SENSOR_T;

#ifndef CMMC_NO_ALIAS
  #define CMMC_Packet CMMC_Packet
#endif

#ifdef ESP8266
  extern "C" {
    #include "user_interface.h"
  }
  #include "ESP8266WiFi.h"
  #include <functional>
#endif

typedef void (*cmmc_debug_cb_t)(const char* message);

class CMMC_Packet
{
    public:
      // constructor
      CMMC_Packet() {
        auto blank = [](const char* message) {};
      }

      ~CMMC_Packet() {}

      void init();
      void debug(cmmc_debug_cb_t);
    private:
      cmmc_debug_cb_t _user_debug_cb;

};

#endif //CMMC_Packet_H
