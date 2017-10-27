#include "CMMC_Blink.h"

void CMMC_Blink::init() {
	pinMode(pin, state);
}

void CMMC_Blink::toggle() {
	state = !state;
	digitalWrite(pin, state);
}
void CMMC_Blink::debug(cmmc_debug_cb_t cb) {
  if (cb != NULL) {
    this->_user_debug_cb = cb;
  }
}

