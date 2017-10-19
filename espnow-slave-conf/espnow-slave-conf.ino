#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <CMMC_Utils.h>
#include <CMMC_SimplePair.h>
#include <CMMC_Config_Manager.h>
#include <CMMC_ESPNow.h>
#include <CMMC_BootMode.h>
#include <CMMC_LED.h>
#include <DHT.h>
#include "FS.h"

#include "data_type.h"

extern "C" {
#include <espnow.h>
#include <user_interface.h>
}

#define LED_PIN 2
#define BUTTON_PIN  13
#define DHTPIN      12
#define DEFAULT_DEEP_SLEEP_S 600


uint8_t master_mac[6];
uint8_t self_mac[6];
int dhtType = 22;
int mode;

CMMC_SimplePair simplePair;
CMMC_Config_Manager configManager;
CMMC_ESPNow espNow;
CMMC_LED led(LED_PIN, HIGH);
DHT dht = DHT(DHTPIN, dhtType);

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
    delay(1000);
    ESP.reset();
  }
  else {
    Serial.printf("[CSP_EVENT_ERROR] %d: %s\r\n", status, (const char*)data);
  }
}


void load_config() {
  configManager.load_config([](JsonObject * root) {
    //    Serial.println("[user] json loaded..");
    if (root->containsKey("mac")) {
      String macStr = String((*root)["mac"].as<const char*>());
      //      Serial.printf("Loaded mac %s\r\n", macStr.c_str());
      CMMC::convertMacStringToUint8(macStr.c_str(), master_mac);
      //      CMMC::printMacAddress(master_mac);
      //      Serial.println();
    }
  });
}
void init_espnow() {
  uint8_t* slave_addr = CMMC::getESPNowSlaveMacAddress();
  memcpy(self_mac, slave_addr, 6);
  espNow.init(NOW_MODE_SLAVE);
  espNow.debug([](const char* c) {
    //    Serial.println(c);
  });
  espNow.on_message_sent([](uint8_t *macaddr, u8 status) {
    led.toggle();
    //    CMMC::printMacAddress(macaddr);
    //    Serial.printf("sent status %lu\r\n", status);
  });

  espNow.on_message_recv([](uint8_t * macaddr, uint8_t * data, uint8_t len) {
    led.toggle();
    //    Serial.printf("GOT sleepTime = %lu\r\n", data[0]);
    if (data[0] == 0) data[0] = 30;
    goSleep(data[0]);
  });
}
void init_simple_pair() {
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
  Serial.begin(115200);
  led.init();
  configManager.init("/config98.json");
  pinMode(5, INPUT_PULLUP);
  uint8_t selective_button_pin = BUTTON_PIN;
  uint32_t wait_button_pin_ms = 10;
  if (digitalRead(5) == LOW) {
    selective_button_pin = 0;
    wait_button_pin_ms = 2000;
  }

  CMMC_BootMode bootMode(&mode, selective_button_pin);

  dht.begin();

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
  //  CMMC::printMacAddress(packet.from);
  //  CMMC::printMacAddress(packet.from);
  //  CMMC::printMacAddress(packet.to);
  packet.sum = CMMC::checksum((uint8_t*) &packet,
                              sizeof(packet) - sizeof(packet.sum));

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    h = 0.0;
    t = 0.0;
    //    Serial.println("Failed to read from DHT sensor!");
  } else {
    packet.temperature = t;
    packet.humidity = h;
  }

  packet.ms = millis();
  //  Serial.printf("%lu - %02x\r\n", packet.battery, packet.battery);
  //  Serial.printf("%lu - %02x\r\n", packet.temperature, packet.temperature);
  //  Serial.printf("%lu - %02x\r\n", packet.humidity, packet.humidity);

}
void loop()
{
  read_sensor();
  auto timeout_cb = []() {
    //    Serial.println("TIMEOUT...");
    goSleep(DEFAULT_DEEP_SLEEP_S);
  };


  if (master_mac[0] == 0x00 && master_mac[1] == 0x00) {
    goSleep(DEFAULT_DEEP_SLEEP_S);
  }
  else {
    espNow.enable_retries(true);
    espNow.send(master_mac, (u8*)&packet, sizeof (packet), timeout_cb, 700);
  }

  delay(100);
}

void goSleep(uint32_t deepSleepS) {
  Serial.printf("\r\nGo sleep for .. %lu seconds. \r\n", deepSleepS);
  ESP.deepSleep(deepSleepS * 10e5);
}

