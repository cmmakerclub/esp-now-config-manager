#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <CMMC_SimplePair.h>
#include <CMMC_Utils.h>
extern "C" {
#include <espnow.h>
#include <user_interface.h>
}

#define LED LED_BUILTIN

CMMC_SimplePair instance;
CMMC_Utils utils;
bool ledState = LOW;

void evt_callback(u8 status, u8* sa, const u8* data) {
  if (status == 0) {
    Serial.printf("[CSP_EVENT_SUCCESS] STATUS: %d\r\n", status);
    Serial.printf("WITH KEY: "); dump(data, 16);
    Serial.printf("WITH MAC: "); dump(sa, 6);
    digitalWrite(LED, HIGH);
    ESP.reset();
  }
  else {
    Serial.printf("[CSP_EVENT_ERROR] %d: %s\r\n", status, (const char*)data);
  }
}

#define BUTTON_PIN 13
void setup()
{
  Serial.begin(115200);
  Serial.flush();

  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  delay(1000);

  if (digitalRead(BUTTON_PIN) == 0) {
    digitalWrite(LED, LOW);
    instance.begin(MASTER_MODE, evt_callback);
    instance.start();
  }
  else {
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    Serial.print("Initializing... Controller..");
    if (esp_now_init() == 0) {
      Serial.print("direct link  init ok");
    } else {
      Serial.print("dl init failed");
      ESP.restart();
      return;
    } 

    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    esp_now_register_recv_cb([](uint8_t *macaddr, uint8_t *data, uint8_t len) {
      //      if (digitalRead(BUTTON_PIN) == HIGH) {
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

void loop()
{

}
