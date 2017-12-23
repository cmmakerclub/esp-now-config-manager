#ifndef CMMC_Utils_H
#define CMMC_Utils_H

#include <Arduino.h>
#include "ESP8266WiFi.h"
#include <functional>

#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#ifdef CMMC_USE_ALIAS
  #define CMMC_Utils CMMC
#endif


class CMMC_Utils
{
public:
    // constructure
    CMMC_Utils() {}
    ~CMMC_Utils() {}

    static void printMacAddress(uint8_t* macaddr, uint8_t newline=0) {
      Serial.print("{");
      for (int i = 0; i < 6; i++) {
        Serial.print("0x");
        Serial.print(macaddr[i], HEX);
        if (i < 5) Serial.print(',');
      }
      Serial.println("};");
    }

    static void dump(const u8* data, size_t size) {
      for (size_t i = 0; i < size; i++) {
        Serial.printf("%02x ", data[i]);
      }
      Serial.println();
    }

    static void convertMacStringToUint8(const char* mac_str, uint8_t* target) {
      String macStr = String(mac_str);
      for (size_t i = 0; i < 12; i += 2) {
        String mac = macStr.substring(i, i + 2);
        byte b = strtoul(mac.c_str(), 0, 16);
        target[i / 2] = b;
      }
    }

    static void macByteToString(const u8* data, char *target) {
       bzero(target, 13);
       sprintf(target, "%02x%02x%02x%02x%02x%02x", data[0], data[1], data[2], data[3], data[4], data[5]);
     }

     static uint8_t* getESPNowControllerMacAddress() {
       static uint8_t _controller_macaddr[6];
       bzero(_controller_macaddr, 6);
       wifi_get_macaddr(STATION_IF, _controller_macaddr);
       return _controller_macaddr;
     }

     static uint8_t* getESPNowSlaveMacAddress() {
       static uint8_t _slave_macaddr[6];
       bzero(_slave_macaddr, 6);
       wifi_get_macaddr(STATION_IF, _slave_macaddr);
       return _slave_macaddr;
     }

    static uint32_t checksum(uint8_t* data, size_t len) {
      uint32_t sum = 0;
      while(len--) {
        sum ^= *(data++);
      }
      return sum;
    }
    static uint32_t unless(uint8_t state, uint32_t a, uint32_t b) {
      if (state == LOW) {
        return a;
      }
      else {
        return b;
      }
    }
   private:
};

#endif //CMMC_Utils_H
