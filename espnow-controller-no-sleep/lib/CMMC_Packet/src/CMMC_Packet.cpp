#include "CMMC_Packet.h"

void CMMC_Packet::init() {

}

void CMMC_Packet::debug(cmmc_debug_cb_t cb) {
  if (cb != NULL) {
    this->_user_debug_cb = cb;
  }
}

