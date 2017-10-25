# CMMC BootMode Library

*A simple boot mode detection for an esp8266*

CMMC BootMode Library is a library for esp8266 that you can define `INPUT_PIN` and `WAIT_TIME` and `CALLBACK` then the `CALLBACK` will be called when checking process is finished

Features
--------
* MIT License

Usage
--------

    int mode;
    int BUTTON_PIN = 0;

    CMMC_BootMode bootMode(&mode, BUTTON_PIN);
    bootMode.check([](int mode) {
        if (mode == BootMode::MODE_CONFIG) {
            // do config_mode
        }
        else if (mode == BootMode::MODE_RUN) {
            // do config_mode
        }
    });
