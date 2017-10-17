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

#define LED 2
#define BUTTON_PIN  0
#define DHTPIN      12

uint8_t master_mac[6];
int dhtType = 22;
int mode;

CMMC_Utils utils;
CMMC_SimplePair instance;
CMMC_Config_Manager configManager;
CMMC_ESPNow espNow;
CMMC_LED led(LED, HIGH);
CMMC_BootMode bootMode(&mode, BUTTON_PIN);
DHT dht = DHT(DHTPIN, dhtType);

void evt_callback(u8 status, u8* sa, const u8* data) {
  if (status == 0) {
    char buf[13];
    Serial.printf("[CSP_EVENT_SUCCESS] STATUS: %d\r\n", status);
    Serial.printf("WITH KEY: "); utils.dump(data, 16);
    Serial.printf("WITH MAC: "); utils.dump(sa, 6);
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

void setup_hardware() {
  Serial.begin(115200);
  Serial.flush();
  WiFi.disconnect(0);

  // Initialize device.
  dht.begin();

  led.init();
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  delay(1000);
}

void load_config() {
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
}

void start_config_mode() {

}

void setup()
{
  setup_hardware();
  load_config();
  bootMode.check([](int mode) {
    if (mode == BootMode::MODE_CONFIG) {
      instance.begin(SLAVE_MODE, evt_callback);
      instance.start();
    }
    else if (mode == BootMode::MODE_RUN) {
      espNow.init(NOW_MODE_SLAVE);
      espNow.on_message_sent([](uint8_t *macaddr, u8 status) {
        led.toggle();
        utils.printMacAddress(macaddr);
        Serial.printf("status %lu\r\n", status);
        if (status == 0) {
          goSleep(10);
        }
      });
    }
    else {
      // unhandled
    }
  });
}

CMMC_SENSOR_T sensor_data;
void loop()
{
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

  espNow.send(master_mac, (u8*)&sensor_data, sizeof (sensor_data));
  utils.dump((u8*)&sensor_data, sizeof (sensor_data));
  delay(1000);
  goSleep(10);
}

void goSleep(uint32_t deepSleepS) {
  Serial.printf("Go sleep for .. %lu seconds. \r\n", deepSleepS);
  ESP.deepSleep(deepSleepS * 10e5);
}

