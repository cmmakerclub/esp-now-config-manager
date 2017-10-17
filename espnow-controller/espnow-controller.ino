#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <CMMC_SimplePair.h>
#include <CMMC_Utils.h>
#include <CMMC_ESPNow.h>
extern "C" {
#include <espnow.h>
#include <user_interface.h>
}

#define LED LED_BUILTIN
#define BUTTON_PIN 13

CMMC_SimplePair instance;
CMMC_ESPNow ESPNow;
CMMC_Utils utils;

bool ledState = LOW;

void evt_callback(u8 status, u8* sa, const u8* data) {
  if (status == 0) {
    Serial.printf("[CSP_EVENT_SUCCESS] STATUS: %d\r\n", status);
    Serial.printf("WITH KEY: ");
    utils.dump(data, 16);
    Serial.printf("WITH MAC: ");
    utils.dump(sa, 6);
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

  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  delay(1000);

}
void check_boot_mode() {
  if (digitalRead(BUTTON_PIN) == 0) {
    digitalWrite(LED, LOW);
    instance.begin(MASTER_MODE, evt_callback);
    instance.start();
  }
  else {
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    delay(50);
    Serial.print("Initializing... Controller..");
    ESPNow.init(NOW_MODE_CONTROLLER);
    ESPNow.on_message([](uint8_t *macaddr, uint8_t *data, uint8_t len) {
      // PACKET_T pkt;
      // memcpy(&pkt, data, sizeof(pkt));
      // memcpy(&pkt.from, macaddr, 48);
      // SENSOR_T sensorData = pkt.data;
      Serial.write(data, len);
      digitalWrite(LED, ledState);
      ledState = !ledState;
    });
  }
}

void setup()
{
  setup_hardware();
  check_boot_mode();
}

void loop()
{

}
