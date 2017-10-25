#ifndef CMMC_BootMode_H
#define CMMC_BootMode_H

#include <Arduino.h>

#ifdef ESP8266
  extern "C" {
    #include "user_interface.h"
  }
  #include "ESP8266WiFi.h"
  #include <functional>
#endif

#ifndef CMMC_NO_ALIAS
  #define CMMC_BootMode BootMode
#endif

typedef void (*cmmc_debug_cb_t)(const char* message);
typedef void (*cmmc_boot_mode_cb_t)(int mode);

class CMMC_BootMode
{
    public:
      static const int MODE_CONFIG = 1;
      static const int MODE_RUN = 2;

      // constructor
      CMMC_BootMode() {
        _user_debug_cb = [](const char* message) {};
        _user_boot_mode_cb = [](int mode) {};
      }

      CMMC_BootMode(int *mode, int button_pin = 0) {
        this->_target_mode = mode;
        this->_button_pin = button_pin;
        CMMC_BootMode();
      }

      ~CMMC_BootMode() {}

      void init();
      void check(cmmc_boot_mode_cb_t mode = NULL, uint32_t wait = 2000);
      void debug(cmmc_debug_cb_t);
    private:
      cmmc_debug_cb_t _user_debug_cb;
      cmmc_boot_mode_cb_t _user_boot_mode_cb;
      int *_target_mode = NULL;
      int _button_pin = 0;
};

#endif //CMMC_BootMode_H
