#define CMMC_USE_ALIAS

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <CMMC_Utils.h>
#include <CMMC_SimplePair.h>
#include <CMMC_Config_Manager.h>
#include <CMMC_ESPNow.h>
#include <CMMC_BootMode.h>
#include <CMMC_LED.h>
#include <CMMC_TimeOut.h>
#include <DHT.h>
#include "FS.h"
#include <CMMC_TimeOut.h>
#include "CMMC_Interval.hpp"
#include "data_type.h"

#include <Adafruit_NeoPixel.h>

#define PIN            13
#define NUMPIXELS      5

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ400);

int delayval = 50; // delay for half a second
extern "C" {
#include <espnow.h>
#include <user_interface.h>
}

#define LED_PIN      2
#define DHTPIN      12
#define DEFAULT_DEEP_SLEEP_S 60

uint8_t selective_button_pin = 13;
uint32_t wait_button_pin_ms = 1;
uint8_t master_mac[6];
uint8_t self_mac[6];
CMMC_TimeOut tm;

int mode;

CMMC_SimplePair simplePair;
CMMC_Config_Manager configManager;
CMMC_ESPNow espNow;
CMMC_LED led(LED_PIN, HIGH);

u8 _pixel_dirty = false;
u8 _pixel_no = 0;
u8 _pixel_r  = 0;
u8 _pixel_g  = 0;
u8 _pixel_b  = 0;
bool sp_flag_done = false;
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
    Serial.println("DONE...");
    sp_flag_done = true;
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
  Serial.print("Slave Mac Address: ");
  CMMC::printMacAddress(self_mac, true);
  espNow.init(NOW_MODE_SLAVE);
  espNow.on_message_sent([](uint8_t *macaddr, u8 status) {
    Serial.printf("sent status %lu\r\n", status);
  });

  espNow.on_message_recv([](uint8_t * macaddr, uint8_t * data, uint8_t len) {
    led.toggle();
    Serial.printf("GOT data  = %u byte\r\n", len);
    _pixel_dirty = true;
    _pixel_no = data[0];
    _pixel_r  = data[1];
    _pixel_g  = data[2];
    _pixel_b  = data[3];
  });
}
void init_simple_pair() {
  simplePair.begin(SLAVE_MODE, evt_callback);
  simplePair.start();
  CMMC_TimeOut ct;
  ct.timeout_ms(3000);
  while (1) {
    if (ct.is_timeout()) {
      if (sp_flag_done && digitalRead(selective_button_pin) == LOW) {
        ct.yield();
      }
      else {
        Serial.println("timeout..........");
        ESP.reset();
      }
    }
    led.toggle();

    delay(50L + (250 * sp_flag_done));
  }
}
void setup()
{
  Serial.begin(57600);
  led.init();
  configManager.init("/config98.json");
  pixels.begin();

  pinMode(5, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);

  selective_button_pin = digitalRead(5) ? 0 : 13;
  wait_button_pin_ms = digitalRead(5) ?  2000 : 1;


  CMMC_BootMode bootMode(&mode, selective_button_pin);
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
  }, wait_button_pin_ms);

  tm.timeout_ms(10);
}

int counter = 0;
CMMC_Interval _1s;
CMMC_Interval interval;
CMMC_SENSOR_T packet;
uint8_t must_send_keep_alive = 0;
void loop()
{
  interval.every_ms(1, []() {
    if (_pixel_dirty) {
      pixels.setPixelColor(_pixel_no, pixels.Color(_pixel_r, _pixel_g, _pixel_b));
      pixels.show();
      _pixel_dirty = false;
    }

    counter++;
  });

  _1s.every_ms(1000, []() {
    packet.battery = analogRead(A0);
    memcpy(packet.to, master_mac, 6);
    memcpy(packet.from, self_mac, 6);
    packet.sum = CMMC::checksum((uint8_t*) &packet,
                                sizeof(packet) - sizeof(packet.sum));
    counter = 0;
    must_send_keep_alive = 1;
  });
  if (must_send_keep_alive) {
    Serial.println(millis());
    espNow.send(master_mac, (u8*)&packet, sizeof (packet));
    must_send_keep_alive = 0;
    delay(10);
  }
}
