#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <CMMC_SimplePair.h>
#include <CMMC_Config_Manager.h>
#include "FS.h"
extern "C" {
#include <espnow.h>
#include <user_interface.h>
}
CMMC_SimplePair instance;
CMMC_Config_Manager configManager;
uint8_t master_mac[6];

#define LED LED_BUILTIN
#define BUTTON_PIN 13

bool ledState = LOW;
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

void evt_callback(u8 status, u8* sa, const u8* data) {
  if (status == 0) {
    Serial.printf("[CSP_EVENT_SUCCESS] STATUS: %d\r\n", status);
    Serial.printf("WITH KEY: "); dump(data, 16);
    Serial.printf("WITH MAC: "); dump(sa, 6);
    char buf[13];
    bzero(buf, 13);
    sprintf(buf, "%02x%02x%02x%02x%02x%02x", data[0], data[1], data[2], data[3], data[4], data[5]);
    Serial.println(buf);
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
      for (size_t i = 0; i < 12; i += 2) {
        String mac = macStr.substring(i, i + 2);
        byte b = strtoul(mac.c_str(), 0, 16);
        master_mac[i / 2] = b;
      }

      printMacAddress(master_mac);
      Serial.println();
    }
  });

  if (digitalRead(BUTTON_PIN) == 0) {
    instance.begin(SLAVE_MODE, evt_callback);
    instance.debug([](const char* s) {
      Serial.printf("[USER]: %s\r\n", s);
    });
//    instance.set_pair_key(0x01);
    instance.start();
  } else { // espnow
    WiFi.disconnect(0);
    WiFi.mode(WIFI_STA);
    delay(100);
    Serial.println("====================");
    Serial.println("   MODE = ESPNOW    ");
    Serial.println("====================");
    WiFi.disconnect();
    Serial.println("Initializing ESPNOW...");
    Serial.println("Initializing... SLAVE");

    if (esp_now_init() == 0) {
      Serial.println("init");
    } else {
      Serial.println("init failed");
      ESP.restart();
      return;
    }
    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_register_send_cb([](uint8_t* macaddr, uint8_t status) {
      Serial.printf("sent status => [%lu]\r\n", status);
      printMacAddress(macaddr);
    });

    while (1) {
      u8 packet[5];
      esp_now_send(master_mac, (u8*) &packet, sizeof(packet));
      digitalWrite(LED_BUILTIN, ledState);
      ledState = !ledState;
      delay(1000);
    }
  }
}

void loop()
{

}
