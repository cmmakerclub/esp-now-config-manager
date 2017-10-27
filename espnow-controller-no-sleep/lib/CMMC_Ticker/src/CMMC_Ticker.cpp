#include "CMMC_Ticker.h"

void CMMC_Ticker::start() {
  this->_ticker->detach();
  this->_ticker->attach_ms(this->_every_ms, this->_ticker_cb);
}
