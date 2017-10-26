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

#include "data_type.h"

extern "C" {
#include <espnow.h>
#include <user_interface.h>
}

#define LED_PIN 2
#define DHTPIN      12
#define DEFAULT_DEEP_SLEEP_S 60

uint8_t selective_button_pin = 13;
uint32_t wait_button_pin_ms = 1;
uint8_t master_mac[6];
uint8_t self_mac[6];
int dhtType = 22;
int mode;

CMMC_SimplePair simplePair;
CMMC_Config_Manager configManager;
CMMC_ESPNow espNow;
CMMC_LED led(LED_PIN, HIGH);
DHT *dht;

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
    led.toggle();
    Serial.println(millis());
    Serial.printf("sent status %lu\r\n", status);
  });

  espNow.on_message_recv([](uint8_t * macaddr, uint8_t * data, uint8_t len) {
    led.toggle();
    Serial.printf("GOT sleepTime = %lu\r\n", data[0]);
    if (data[0] == 0) data[0] = 30;
    goSleep(data[0]);
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
  pinMode(5, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);

  selective_button_pin = digitalRead(5) ? 13 : 0;
  wait_button_pin_ms = digitalRead(5) ?  1 : 2000;
  dhtType = digitalRead(4) ? 22 : 11;

  dht = new DHT(DHTPIN, dhtType);
  dht->begin();

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
}

CMMC_SENSOR_T packet;

void read_sensor() {
  packet.battery = analogRead(A0);
  memcpy(packet.to, master_mac, 6);
  memcpy(packet.from, self_mac, 6);
  //CMMC::printMacAddress(packet.from);
  //CMMC::printMacAddress(packet.to);
  packet.sum = CMMC::checksum((uint8_t*) &packet,
                              sizeof(packet) - sizeof(packet.sum));

  float h = dht->readHumidity();
  float t = dht->readTemperature();
  if (isnan(h) || isnan(t)) {
    h = 0.0;
    t = 0.0;
  } else {
    packet.temperature = t * 100;
    packet.humidity = h * 100;
  }

  packet.ms = millis();
  //  Serial.printf("%lu - %02x\r\n", packet.battery, packet.battery);
  //  Serial.printf("%lu - %02x\r\n", packet.temperature, packet.temperature);
  //  Serial.printf("%lu - %02x\r\n", packet.humidity, packet.humidity);
}

auto timeout_cb = []() {
  Serial.println("TIMEOUT...");
  goSleep(DEFAULT_DEEP_SLEEP_S);
};

void loop()
{
  read_sensor();

  if (master_mac[0] == 0x00 && master_mac[1] == 0x00) {
    goSleep(DEFAULT_DEEP_SLEEP_S);
  }
  else {
    espNow.enable_retries(true);
    Serial.println(millis());
    espNow.send(master_mac, (u8*)&packet, sizeof (packet), timeout_cb, 3000);
  }

  delay(100);
}

void goSleep(uint32_t deepSleepS) {
  //Serial.printf("\r\nGo sleep for .. %lu seconds. \r\n", deepSleepS);
  ESP.deepSleep(deepSleepS * 1e6);
}
