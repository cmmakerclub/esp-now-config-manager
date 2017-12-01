#define CMMC_USE_ALIAS
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BME280.h>

#define LED_PIN 2
#define PROD_MODE_PIN         13
#define BUTTON_PIN             0
#define DEFAULT_DEEP_SLEEP_S 60

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

#define SEALEVELPRESSURE_HPA (1013.25) 

Adafruit_BME280 bme; // I2C 
CMMC_LED led(LED_PIN, HIGH); 
CMMC_SimplePair simplePair;
CMMC_Config_Manager configManager;
CMMC_ESPNow espNow;


uint8_t master_mac[6];
uint8_t self_mac[6];
int mode;

#include "sp.h" 

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
    //    goSleep(120);
  });

  espNow.on_message_recv([](uint8_t * macaddr, uint8_t * data, uint8_t len) {
    led.toggle();
    Serial.printf("GOT sleepTime = %lu\r\n", data[0]);
    if (data[0] == 0) 
      data[0] = 30;
    goSleep(data[0]);
  });
} 

void setup()
{
  Serial.begin(57600);
  Serial.println("starting...");
  led.init();
  Wire.begin();
  bme.begin(0x76);
  configManager.init("/config98.json");
  pinMode(PROD_MODE_PIN, INPUT_PULLUP); 
  uint32_t wait_config = 1000;
  if (digitalRead(PROD_MODE_PIN) == LOW) {
    wait_config = 0; 
  } 

  CMMC_BootMode bootMode(&mode, BUTTON_PIN);

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
  }, wait_config);
}

CMMC_SENSOR_T packet;

void read_sensor() {
  packet.type = 1;
  packet.battery = analogRead(A0);
  memcpy(packet.to, master_mac, 6);
  memcpy(packet.from, self_mac, 6); 
  strcpy(packet.myName, "BME280-ID-01");
  packet.nameLen = strlen(packet.myName); 

  bool read_ok = 0;
  while(!read_ok) {
    float t = bme.readTemperature();
    float h = bme.readHumidity();
    Serial.println(h);

    if (isnan(h) || h == 0) {
      Serial.println("read bme280 failed... try again..");
      delay(1000); 
    }
    else {
      packet.ms = millis();
      packet.sent_ms = millis();
      packet.field1 = t * 100;
      packet.field2 = h * 100;
      packet.field3 = bme.readPressure(); 
      break;
    }
  }
  packet.sum = CMMC::checksum((uint8_t*) &packet,
                              sizeof(packet) - sizeof(packet.sum));

   Serial.printf("battery     %lu(%02x)\r\n", packet.battery, packet.battery);
   Serial.printf("temperature %lu(%02x)\r\n", packet.field1, packet.field1);
   Serial.printf("humidity    %lu(%02x)\r\n", packet.field2, packet.field2);
   Serial.printf("pressure    %lu(%02x)\r\n", packet.field3, packet.field3);
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
    espNow.send(master_mac, (u8*)&packet, sizeof (packet), timeout_cb, 5000);
  }

  delay(100);
}

void goSleep(uint32_t deepSleepS) {
  Serial.printf("\r\nGo sleep for .. %lu seconds. \r\n", deepSleepS);
  ESP.deepSleep(deepSleepS * 1e6);
}
