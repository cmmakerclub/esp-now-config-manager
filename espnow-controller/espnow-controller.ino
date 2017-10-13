#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <CMMC_SimplePair.h>
extern "C" {
#include <espnow.h>
#include <user_interface.h>
}

CMMC_SimplePair instance;
bool ledState = LOW;

void dump(const u8* data, size_t size) {
  for (size_t i = 0; i < size - 1; i++) {
    Serial.printf("%02x ", data[i]);
  }
  Serial.printf("%02x", data[size - 1]);
  Serial.println();
}

void evt_success(u8* sa, u8 status, const u8* key) {
  Serial.printf("[CSP_EVENT_SUCCESS] STATUS: %d\r\n", status);
  Serial.printf("WITH KEY: "); dump(key, 16);
  Serial.printf("WITH MAC: "); dump(sa, 6);
  digitalWrite(LED_BUILTIN, HIGH);
  ESP.reset();
}

void evt_error(u8* sa, u8 status, const char* cause) {
  Serial.printf("[CSP_EVENT_ERROR] %d: %s\r\n", status, cause);
}

u8 pair_key[16] = {0x09, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                   0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
                  };
u8 message[16] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xff, 0xfa
                 };

#define BUTTON_PIN 13
void setup()
{
  Serial.begin(115200);
  Serial.flush();

  pinMode(LED_BUILTIN, OUTPUT); 
  digitalWrite(LED_BUILTIN, HIGH);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  delay(1000);

  if (digitalRead(BUTTON_PIN) == 0) {
    digitalWrite(LED_BUILTIN, LOW);
    WiFi.disconnect(0);
    delay(100);
    WiFi.mode(WIFI_AP);
    WiFi.softAP("hello nazt");

    wifi_get_macaddr(STATION_IF, message);
    instance.begin(MASTER_MODE, pair_key, message, evt_success, evt_error);
    instance.add_debug_listener([](const char* s) {
      Serial.printf("[USER]: %s\r\n", s);
    });
    instance.start();
  }
  else {
    Serial.print("Initializing... Controller..");
    WiFi.disconnect(0);
    delay(100);
    WiFi.mode(WIFI_STA);
    uint8_t macaddr[6];

    wifi_get_macaddr(STATION_IF, macaddr);
    Serial.print("[master] address (STATION_IF): ");
    printMacAddress(macaddr);
    Serial.println();

    wifi_get_macaddr(SOFTAP_IF, macaddr);
    Serial.print("[slave] address (SOFTAP_IF): ");
    printMacAddress(macaddr);
    Serial.println();


    if (esp_now_init() == 0) {
      Serial.print("direct link  init ok");
    } else {
      Serial.print("dl init failed");
      ESP.restart();
      return;
    }

    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    esp_now_register_recv_cb([](uint8_t *macaddr, uint8_t *data, uint8_t len) {
      //      freqCounter++;
      //      if (digitalRead(BUTTON_PIN) == HIGH) {
      // PACKET_T pkt;
      // memcpy(&pkt, data, sizeof(pkt));
      // memcpy(&pkt.from, macaddr, 48);
      // SENSOR_T sensorData = pkt.data;
      Serial.write(data, len);
      //}
      digitalWrite(LED_BUILTIN, ledState);
      ledState = !ledState;

    });
  }

}

void loop()
{

}

void printMacAddress(uint8_t* macaddr) {
  Serial.print("{");
  for (int i = 0; i < 6; i++) {
    Serial.print("0x");
    Serial.print(macaddr[i], HEX);
    if (i < 5) Serial.print(',');
  }
  Serial.println("};");
}
