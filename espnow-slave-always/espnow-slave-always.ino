#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <CMMC_Utils.h>
#include <CMMC_SimplePair.h>
#include <CMMC_Config_Manager.h>
#include <CMMC_ESPNow.h>
#include <CMMC_BootMode.h>
#include <CMMC_LED.h>
#include <CMMC_Ticker.h>
#include "FS.h"

#include "data_type.h"

extern "C" {
#include <espnow.h>
#include <user_interface.h>
}

#define LED_PIN 2
#define BUTTON_PIN  0

uint8_t master_mac[6];
uint8_t self_mac[6];
int mode;

CMMC_SimplePair simplePair;
CMMC_Config_Manager configManager;
CMMC_ESPNow espNow;
CMMC_LED led(LED_PIN, HIGH);
CMMC_Ticker ticker;

void evt_callback(u8 status, u8* sa, const u8* data) {
  if (status == 0) {
    char buf[13];
    Serial.printf("[CSP_EVENT_SUCCESS] STATUS: %d\r\n", status);
    Serial.printf("WITH KEY: ");
    CMMC::dump(data, 16);
    Serial.printf("WITH MAC: ");
    CMMC::dump(sa, 6);
    CMMC::macByteToString(data, buf);
    CMMC::printMacAddress((uint8_t*)buf);
    configManager.add_field("mac", buf);
    configManager.commit();
    led.high();
    delay(5000);
    ESP.reset();
  }
  else {
    Serial.printf("[CSP_EVENT_ERROR] %d: %s\r\n", status, (const char*)data);
  }
}


void load_config() {
  configManager.load_config([](JsonObject * root) {
    Serial.println("[user] json loaded..");
    if (root->containsKey("mac")) {
      String macStr = String((*root)["mac"].as<const char*>());
      Serial.printf("Loaded mac %s\r\n", macStr.c_str());
      CMMC::convertMacStringToUint8(macStr.c_str(), master_mac);
      CMMC::printMacAddress(master_mac);
      Serial.println();
    }
  });
}
void init_espnow() {
  uint8_t* slave_addr = CMMC::getESPNowSlaveMacAddress();
  memcpy(self_mac, slave_addr, 6);
  espNow.init(NOW_MODE_SLAVE);
  espNow.debug([](const char* c) {
    Serial.println(c);
  });
  espNow.on_message_sent([](uint8_t *macaddr, u8 status) {
    led.toggle();
    Serial.println(millis());
    //    CMMC::printMacAddress(macaddr);
    Serial.printf("sent status %lu\r\n", status);
  });

  espNow.on_message_recv([](uint8_t * macaddr, uint8_t * data, uint8_t len) {
    led.toggle();
  });

  ticker.start();
}
void init_simple_pair() {
  simplePair.debug([](const char* c) {
    Serial.println(c);
  });
  simplePair.begin(SLAVE_MODE, evt_callback);
  simplePair.debug([](const char* c) {
    Serial.println(c);
  });
  simplePair.start();
  while (1) {
    yield();
  }
}
void setup()
{


  Serial.begin(57600);
  Serial.println("HELLO");
  SPIFFS.begin();

  led.init();
  led.low();
  delay(1000);
  configManager.add_debug_listener([](const char* c) {
    Serial.println(c);
  });
  configManager.init("/config98.json");
  CMMC_BootMode bootMode(&mode, 0);

  bootMode.init();
  bootMode.check([](int mode) {
    if (mode == BootMode::MODE_CONFIG) {
      init_simple_pair();
    }
    else if (mode == BootMode::MODE_RUN) {
      load_config();
      init_espnow();
    }
    else {
      // unhandled
    }
  }, 2000);
}



void send_keep_alive() {
  CMMC_SENSOR_T packet;
  packet.battery = analogRead(A0);
  memcpy(packet.to, master_mac, 6);
  memcpy(packet.from, self_mac, 6);
  packet.sum = CMMC::checksum((uint8_t*) &packet,
                              sizeof(packet) - sizeof(packet.sum));
  packet.ms = millis();
  send(master_mac, (u8*)&packet);

}

void send(uint8_t *mac, u8* packet) {
  static auto timeout_cb = []() {
    Serial.println("SEND TIMEOUT...");
  };

  if (mac[0] == 0x00 && mac[1] == 0x00) {
    Serial.println("Invalid target");
  }
  else {
    espNow.enable_retries(true);
    Serial.println(millis());
    espNow.send(mac, (u8*)&packet, sizeof (*packet), timeout_cb, 2000);
  }

}

void loop()
{
  if (ticker.is_dirty()) {
    Serial.println("Ticker.tick");
    send_keep_alive();
    ticker.clear_dirty();
  }

  Serial.println("HELLO...");
  delay(500);

}
