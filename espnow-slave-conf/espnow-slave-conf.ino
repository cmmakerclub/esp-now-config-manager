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
#define BUTTON_PIN  0
#define DHTPIN      12
#define DEFAULT_DEEP_SLEEP_S 30
uint8_t master_mac[6];
int dhtType = 22;
int mode;

CMMC_Utils utils;
CMMC_SimplePair simplePair;
CMMC_Config_Manager configManager;
CMMC_ESPNow espNow;
CMMC_LED led(LED_PIN, HIGH);
CMMC_BootMode bootMode(&mode, BUTTON_PIN);
DHT dht = DHT(DHTPIN, dhtType);

void evt_callback(u8 status, u8* sa, const u8* data) {
  if (status == 0) {
    char buf[13];
    Serial.printf("[CSP_EVENT_SUCCESS] STATUS: %d\r\n", status);
    Serial.printf("WITH KEY: "); utils.dump(data, 16);
    Serial.printf("WITH MAC: "); utils.dump(sa, 6);
    utils.macByteToString(data, buf);
    utils.printMacAddress((uint8_t*)buf);
    configManager.add_debug_listener([](const char* msg) {
      Serial.println(msg);
    });
    configManager.add_field("mac", buf);
    configManager.commit();
    led.high();
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
      utils.convertMacStringToUint8(macStr.c_str(), master_mac);
      utils.printMacAddress(master_mac);
      Serial.println();
    }
  });
}
void init_espnow() {
  espNow.init(NOW_MODE_SLAVE);
  espNow.on_message_sent([](uint8_t *macaddr, u8 status) {
    led.toggle();
    utils.printMacAddress(macaddr);
    Serial.printf("status %lu\r\n", status);
  });

  espNow.on_message_recv([](uint8_t * macaddr, uint8_t * data, uint8_t len) {
    Serial.printf("GOT sleepTime = %lu\r\n", data[0]);
    if (data[0] == 0) data[0] = 30;
    goSleep(data[0]);
  });
}
void init_simple_pair() {
  simplePair.begin(SLAVE_MODE, evt_callback);
  simplePair.start();
  while (1) {
    yield();
  }
}
void setup()
{
  Serial.begin(115200);
  Serial.flush();
  Serial.println();
  dht.begin();
  led.init();
  SPIFFS.begin();
  configManager.init("/config2.json");
  bootMode.init();
  bootMode.check([](int mode) {
    Serial.println(mode);
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
  }, 500);
}

CMMC_SENSOR_T sensor_data;

void read_sensor() {
  sensor_data.battery = analogRead(A0);
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    h = 0.0;
    t = 0.0;
  } else {
    sensor_data.temperature = t * 1000;
    sensor_data.humidity = h * 1000;
  }

  sensor_data.ms = millis();
  Serial.printf("%lu - %02x\r\n", sensor_data.battery, sensor_data.battery);
  Serial.printf("%lu - %02x\r\n", sensor_data.temperature, sensor_data.temperature);
  Serial.printf("%lu - %02x\r\n", sensor_data.humidity, sensor_data.humidity);
}
void loop()
{
  read_sensor();
  uint32_t dataHasBeenSentAtMillis = millis();
  espNow.send(master_mac, (u8*)&sensor_data, sizeof (sensor_data));
  utils.dump((u8*)&sensor_data, sizeof (sensor_data));

  while (true) {
    Serial.println("Waiting a command message...");
    if (millis() > (dataHasBeenSentAtMillis + 500L)) {
      Serial.println("TIMEOUT!!!!");
      Serial.println("go to bed!");
      Serial.println("....BYE");
      goSleep(DEFAULT_DEEP_SLEEP_S);
    }
    delay(100);
  }
}

void goSleep(uint32_t deepSleepS) {
  Serial.printf("Go sleep for .. %lu seconds. \r\n", deepSleepS);
  ESP.deepSleep(deepSleepS * 10e5);
}

