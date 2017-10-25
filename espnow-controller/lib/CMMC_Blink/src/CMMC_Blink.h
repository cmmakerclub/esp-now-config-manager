#ifndef CMMC_Blink_H
#define CMMC_Blink_H

#include <Arduino.h>


#ifndef CMMC_NO_ALIAS
  #define CMMC_Blink CMMC_Blink
#endif

#ifdef ESP8266
  extern "C" {
    #include "user_interface.h"
  }
  #include "ESP8266WiFi.h"
  #include <functional>
#endif

typedef void (*cmmc_debug_cb_t)(const char* message);

class CMMC_Blink
{
    public:
      // constructor
      CMMC_Blink(int p, int s) {
        pin = p;
        state = s;
      }

      ~CMMC_Blink() {}

      void toggle();
      void init();
      void debug(cmmc_debug_cb_t);
      int state;
    private:
      int pin;

      cmmc_debug_cb_t _user_debug_cb;

};

#endif //CMMC_Blink_H
