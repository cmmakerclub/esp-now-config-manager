#ifndef CMMC_ESP8266_Utils_H
#define CMMC_ESP8266_Utils_H

#include "ESP8266WiFi.h"
#include <functional>

#ifdef ESP8266
extern "C" {
  #include "user_interface.h"
}
#endif

class CMMC_ESP8266_Utils
{
public:

    // constructure
    CMMC_ESP8266_Utils() {}
    ~CMMC_ESP8266_Utils() {}

    void setup();

};

#endif //CMMC_ESP8266_Utils_H
