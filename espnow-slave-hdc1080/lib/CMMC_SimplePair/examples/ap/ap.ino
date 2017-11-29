#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <CMMC_SimplePair.h>

CMMC_SimplePair instance;

void dump(const u8* data, size_t size) {
  for (size_t i = 0; i < size-1; i++) {
    Serial.printf("%02x ", data[i]);
  }
  Serial.printf("%02x", data[size-1]);
  Serial.println();
}

void evt_callback(u8 status, u8* sa, const u8* data) {
  if (status == 0) {
    Serial.printf("[CSP_EVENT_SUCCESS] STATUS: %d\r\n", status);
    Serial.printf("WITH KEY: "); dump(data, 16);
    Serial.printf("WITH MAC: "); dump(sa, 6);
  }
  else {
    Serial.printf("[CSP_EVENT_ERROR] %d: %s\r\n", status, (const char*)data);
  }
}

void setup()
{
  Serial.begin(115200);
  instance.begin(MASTER_MODE, evt_callback);
  instance.debug([](const char* s) {
    Serial.printf("[USER]: %s\r\n", s);
  });
  instance.start();
}

void loop()
{

}
