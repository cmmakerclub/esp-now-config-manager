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

void init_simple_pair() {
  simplePair.begin(SLAVE_MODE, evt_callback);
  simplePair.start();
  CMMC_TimeOut ct;
  ct.timeout_ms(3000);
  while (1) {
    if (ct.is_timeout()) {
      if (sp_flag_done && digitalRead(BUTTON_PIN) == LOW) {
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