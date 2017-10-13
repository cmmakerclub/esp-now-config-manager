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
u8 pair_key[16] = {0x09, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                   0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
                  };

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

void evt_success(u8* sa, u8 status, const u8* key) {
  Serial.printf("[CSP_EVENT_SUCCESS] STATUS: %d\r\n", status);
  Serial.printf("WITH KEY: "); dump(key, 16);
  Serial.printf("WITH MAC: "); dump(sa, 6);
  char buf[13];
  bzero(buf, 13);
  sprintf(buf, "%02x%02x%02x%02x%02x%02x",
          key[0], key[1], key[2], key[3], key[4], key [5]);
  Serial.println(buf);
  configManager.add_field("mac", buf);

  configManager.commit();
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  ESP.reset();
}

void evt_error(u8* sa, u8 status, const char* cause) {
  Serial.printf("[CSP_EVENT_ERROR] %d: %s\r\n", status, cause);
}

uint8_t master_mac[6];
#define BUTTON_PIN 13
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
    WiFi.disconnect(0);
    delay(100);
    WiFi.mode(WIFI_STA);
    delay(100);
    instance.begin(SLAVE_MODE, pair_key, NULL, evt_success, evt_error);
    instance.add_debug_listener([](const char* s) {
      Serial.printf("[USER]: %s\r\n", s);
    });
    instance.start();
  } else { // espnow
    WiFi.disconnect(0);
    delay(100);
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
