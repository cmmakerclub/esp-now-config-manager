#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <CMMC_Config_Manager.h>
#include "FS.h"

CMMC_Config_Manager configManager;

void setup()
{
  Serial.begin(115200);
  
  SPIFFS.begin();
  configManager.add_debug_listener([](const char* s) { });

  configManager.init("/apconfig.json");
  configManager.add_field("wifi", "nat");
  configManager.add_field("ssid", "p12345678");
  configManager.add_field("mac", "062696d86551");
  configManager.commit();

  configManager.load_config([](JsonObject *root) {
    Serial.println("[user] json loaded..");
    root->printTo(Serial);
    Serial.println();
  });
}

void loop()
{
}
