#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <CMMC_SimplePair.h>
#include <CMMC_Utils.h>
#include <CMMC_ESPNow.h>
#include <CMMC_LED.h>
#include <CMMC_BootMode.h>
#include "data_type.h"

#define LED_PIN 2
#define BUTTON_PIN 0

int mode;

CMMC_SimplePair instance;
CMMC_ESPNow espNow;
CMMC_Utils utils;
CMMC_LED led(LED_PIN, HIGH);
CMMC_BootMode bootMode(&mode, BUTTON_PIN);

void evt_callback(u8 status, u8* sa, const u8* data) {
  if (status == 0) {
    Serial.printf("[CSP_EVENT_SUCCESS] STATUS: %d\r\n", status);
    Serial.printf("WITH KEY: ");
    utils.dump(data, 16);
    Serial.printf("WITH MAC: ");
    utils.dump(sa, 6);
    led.high();
    ESP.reset();
  }
  else {
    Serial.printf("[CSP_EVENT_ERROR] %d: %s\r\n", status, (const char*)data);
  }
}
void setup_hardware() {
  Serial.begin(115200);
  Serial.flush();
  led.init();
}

void start_config_mode() {
  uint8_t* controller_addr = utils.getESPNowControllerMacAddress();
  utils.printMacAddress(controller_addr);
  instance.begin(MASTER_MODE, evt_callback);
  instance.set_message(controller_addr, 6);
  instance.start();
}

void setup()
{
  setup_hardware();
  Serial.println();
  bootMode.init();
  bootMode.check([](int mode) {
    if (mode == BootMode::MODE_CONFIG) {
      led.low();
      start_config_mode();
    }
    else if (mode == BootMode::MODE_RUN) {
      Serial.print("Initializing... Controller..");
      espNow.init(NOW_MODE_CONTROLLER);
      espNow.on_message_recv([](uint8_t *macaddr, uint8_t *data, uint8_t len) {
        led.toggle();
        utils.printMacAddress(macaddr);
        Serial.println();
        CMMC_SENSOR_T sensor_data;
        memcpy(&sensor_data, data, sizeof(sensor_data));
        Serial.printf("millis() = %lu \r\n", millis());
        Serial.printf("batt: %lu - %02x\r\n", sensor_data.battery, sensor_data.battery);
        Serial.printf("temp: %0.2f - %02x\r\n", sensor_data.temperature/1000.0, sensor_data.temperature);
        Serial.printf("humid: %0.2f - %02x\r\n", sensor_data.humidity/1000.0, sensor_data.humidity);
        u8 b = 2;
        espNow.send(macaddr, &b, 1);
      });

      espNow.on_message_sent([](uint8_t *macaddr,  uint8_t status) {
         Serial.printf("status = %lu\r\n", status);
      });
    }
    else {
      // unhandled
    }
  });
}

void loop()
{

}
