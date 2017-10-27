#include "CMMC_TimeOut.h"

void CMMC_TimeOut::timeout_ms(uint32_t t) {
    this->_user_timeout_ms = t;
    this->_next_tick = millis() + _user_timeout_ms; 
}

bool CMMC_TimeOut::is_timeout() {
    return (millis() > this->_next_tick);
} 
void CMMC_TimeOut::yield() {
    this->_next_tick = millis() + _user_timeout_ms; 
}