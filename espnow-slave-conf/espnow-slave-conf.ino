#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <CMMC_Utils.h>
#include <CMMC_SimplePair.h>
#include <CMMC_Config_Manager.h>
#include <CMMC_ESPNow.h>
#include "FS.h"
extern "C" {
#include <espnow.h>
#include <user_interface.h>
}

CMMC_Utils utils;
CMMC_SimplePair instance;
CMMC_Config_Manager configManager;
CMMC_ESPNow ESPNow;

uint8_t master_mac[6];

#define LED LED_BUILTIN
#define BUTTON_PIN 13

bool ledState = LOW;

void evt_callback(u8 status, u8* sa, const u8* data) {
  if (status == 0) {
    Serial.printf("[CSP_EVENT_SUCCESS] STATUS: %d\r\n", status);
    Serial.printf("WITH KEY: ");
    utils.dump(data, 16);
    Serial.printf("WITH MAC: ");
    utils.dump(sa, 6);

    char buf[13];
    utils.macByteToString(data, buf);

    configManager.add_field("mac", buf);
    configManager.commit();

    digitalWrite(LED, HIGH);
    ESP.reset();
  }
  else {
    Serial.printf("[CSP_EVENT_ERROR] %d: %s\r\n", status, (const char*)data);
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.flush();
  WiFi.disconnect(0);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  delay(1000);

  SPIFFS.begin();
  configManager.init("/config2.json");
  configManager.load_config([](JsonObject * root) {
    Serial.println("[user] json loaded..");
    if (root->containsKey("mac")) {
      String macStr = String((*root)["mac"].as<const char*>());
      Serial.printf("Loaded mac %s\r\n", macStr.c_str());
      utils.convertMacStringToUint8(macStr.c_str(), master_mac);
      utils.printMacAddress(master_mac);
      Serial.println();
    }
  });

  if (digitalRead(BUTTON_PIN) == 0) {
    instance.begin(SLAVE_MODE, evt_callback);
    instance.debug([](const char* s) {
      Serial.printf("[USER]: %s\r\n", s);
    });
    instance.start();
  } else { 
    // espnow
    ESPNow.init(NOW_MODE_SLAVE);
    u8 packet[5];
    while (1) {
      ESPNow.send(master_mac, packet, sizeof packet);
      digitalWrite(LED_BUILTIN, ledState);
      ledState = !ledState;
      delay(1000);
    }
  }
}

void loop()
{

}
