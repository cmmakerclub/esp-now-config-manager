#include "CMMC_BootMode.h"

void CMMC_BootMode::init() {
  pinMode(this->_button_pin, INPUT_PULLUP);
}

void CMMC_BootMode::check(cmmc_boot_mode_cb_t cb, uint32_t wait) {
  delay(wait);
  if (cb != NULL) this->_user_boot_mode_cb = cb;

  if (digitalRead(this->_button_pin) == LOW) {
    *_target_mode  = MODE_CONFIG;
  }
  else {
    *_target_mode  = MODE_RUN;
  }

  // finally fire the cb
  _user_boot_mode_cb(*_target_mode);
}

void CMMC_BootMode::debug(cmmc_debug_cb_t cb) {
  if (cb != NULL) {
    this->_user_debug_cb = cb;
  }
}
