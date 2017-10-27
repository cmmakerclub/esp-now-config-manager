#include "CMMC_LED.h"

void CMMC_LED::init() {
  pinMode(this->_led, OUTPUT);
  digitalWrite(this->_led, this->_led_state);
}

void CMMC_LED::toggle() {
    this->_led_state = !this->_led_state;
    digitalWrite(this->_led, this->_led_state);
}

void CMMC_LED::debug(cmmc_debug_cb_t cb) {
  if (cb != NULL) {
    this->_user_debug_cb = cb;
  }
}
