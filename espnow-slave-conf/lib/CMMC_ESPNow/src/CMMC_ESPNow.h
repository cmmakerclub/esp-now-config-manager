#ifndef CMMC_ESPNow_H
#define CMMC_ESPNow_H

#ifdef ESP8266
  extern "C" {
    #include "user_interface.h"
    #include "espnow.h"
  } 
  #include "ESP8266WiFi.h"
  #include <functional>
#endif

#define NOW_MODE_SLAVE 1
#define NOW_MODE_CONTROLLER 2

#define USER_DEBUG_PRINTF(fmt, args...) { \
    sprintf(this->debug_buffer, fmt, ## args); \
    _user_debug_cb(this->debug_buffer); \
  }

typedef void (*cmmc_debug_cb_t)(const char* message);
typedef void (*void_cb_t)();

class CMMC_ESPNow
{
  public:
    // constructor
    CMMC_ESPNow(); 
    ~CMMC_ESPNow() {}

    void init(int mode);
    void enable_retries() { }
    void send(uint8_t *mac, u8* data, int len, void_cb_t cb = NULL, uint32_t wait_time = 500);
    void on_message_recv(esp_now_recv_cb_t cb); 
    void on_message_sent(esp_now_send_cb_t cb); 
    void debug(cmmc_debug_cb_t cb); 
    void enable_retries(bool s); 
  private:
    // callbacks
    cmmc_debug_cb_t _user_debug_cb;
    esp_now_recv_cb_t _system_on_message_recv;
    esp_now_send_cb_t _system_on_message_sent; 
    esp_now_recv_cb_t _user_on_message_recv;
    esp_now_send_cb_t _user_on_message_sent;
    // attributes
    uint8_t _message_sent_status;
    bool _enable_retries = false;
    bool _waiting_message_has_arrived;
    char debug_buffer[60];
};

#endif //CMMC_ESPNow_H
