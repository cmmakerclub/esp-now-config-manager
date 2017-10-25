#ifndef CMMC_Ticker_H
#define CMMC_Ticker_H

#include <Arduino.h>
#include <Ticker.h>

typedef void (*cmmc_ticker_cb_t)(void);
class CMMC_Ticker
{
    public:
      // constructor
      CMMC_Ticker(uint32_t every_ms = 1000, uint8_t *state_ptr = NULL) {
        static CMMC_Ticker *that = this;
        this->_user_state_ptr = state_ptr;
        this->_every_ms = every_ms;
        this->_ticker = new Ticker;
        this->_ticker_cb = []() {
          that->_dirty_flag = 1; 
          if (that->_user_state_ptr != NULL) {
            *that->_user_state_ptr = that->_dirty_flag; 
          }
        };
      }
      ~CMMC_Ticker() {} 
      uint8_t is_dirty() {
        return _dirty_flag;
      }
      void clear_dirty() {
        _dirty_flag = 0;
        if (_user_state_ptr != NULL) {
          *_user_state_ptr = _dirty_flag;
        }
      }
      void start(); 
    private:
      uint8_t *_user_state_ptr = NULL;
      uint8_t _dirty_flag = 0; 
      uint32_t _every_ms;
      Ticker *_ticker = NULL;
      cmmc_ticker_cb_t _ticker_cb;
};

#endif //CMMC_Ticker_H
