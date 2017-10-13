#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <CMMC_SimplePair.h>
#include <CMMC_Config_Manager.h>
#include "FS.h"

CMMC_SimplePair instance;
CMMC_Config_Manager configManager;
u8 pair_key[16] = {0x09, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                   0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
                  };
void printMacAddress(uint8_t* macaddr) {
  Serial.print("{");
  for (int i = 0; i < 6; i++) {
    Serial.print("0x");
    Serial.print(macaddr[i], HEX);
    if (i < 5) Serial.print(',');
  }
  Serial.println("};");
}

void dump(const u8* data, size_t size) {
  for (size_t i = 0; i < size - 1; i++) {
    Serial.printf("%02x ", data[i]);
  }
  Serial.printf("%02x", data[size - 1]);
  Serial.println();
}

void evt_success(u8* sa, u8 status, const u8* key) {
  Serial.printf("[CSP_EVENT_SUCCESS] STATUS: %d\r\n", status);
  Serial.printf("WITH KEY: "); dump(key, 16);
  Serial.printf("WITH MAC: "); dump(sa, 6);
  char buf[13];
  bzero(buf, 13);
  sprintf(buf, "%02x%02x%02x%02x%02x%02x",
          sa[0], sa[1], sa[2], sa[3], sa[4], sa[5]);
  Serial.println(buf);
  configManager.add_field("mac", buf);

  configManager.commit();

}

void evt_error(u8* sa, u8 status, const char* cause) {
  Serial.printf("[CSP_EVENT_ERROR] %d: %s\r\n", status, cause);
}

uint8_t master_mac[6];

void setup()
{
  Serial.begin(115200);
  SPIFFS.begin();
  configManager.init("/config.json");


  configManager.load_config([](JsonObject * root) {
    Serial.println("[user] json loaded..");
    if (root->containsKey("mac")) {
      String macStr = String((*root)["mac"].as<const char*>());
      Serial.printf("Loaded mac %s\r\n", macStr.c_str());
      for (size_t i = 0; i < 12; i += 2) {
        String mac = macStr.substring(i, i + 2);
        byte b = strtoul(mac.c_str(), 0, 16);
        master_mac[i / 2] = b;
      }

      printMacAddress(master_mac);
      Serial.println();
    }
  });

  instance.begin(SLAVE_MODE, pair_key, NULL, evt_success, evt_error);
  instance.add_debug_listener([](const char* s) {
    Serial.printf("[USER]: %s\r\n", s);
  });
  instance.start();

}

void loop()
{

}
