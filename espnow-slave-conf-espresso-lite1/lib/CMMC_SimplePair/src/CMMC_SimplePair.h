#ifndef CMMC_SimplePair_H
#define CMMC_SimplePair_H

#include "ESP8266WiFi.h"
#include <functional>

#ifdef ESP8266
extern "C" {
#include "user_interface.h"
  #include "user_interface.h"
  #include "simple_pair.h"
}
#endif

enum CMMC_SimplePair_mode_t {
    CSP_MODE_AP, CSP_MODE_STA
};
enum CMMC_SimplePair_event_t {
    CSP_EVENT_SUCCESS, CSP_EVENT_ERROR
};

#ifndef CSP_DEBUG_BUFFER
  #define CSP_DEBUG_BUFFER 120
#endif

#define MASTER_MODE CSP_MODE_AP
#define SLAVE_MODE CSP_MODE_STA
#define EVENT_SUCCESS CSP_EVENT_SUCCESS
#define EVENT_ERROR CSP_EVENT_ERROR

typedef void (*cmmc_simple_pair_status_cb_t)(u8 status, u8 *sa, const u8* cause);
typedef void (*cmmc_debug_cb_t)(const char* msg);

class CMMC_SimplePair
{
  public:
      // constructure
      ~CMMC_SimplePair() {}
      CMMC_SimplePair() {
        auto cmmc_err_blank = [](u8 status, u8 *sa, const u8* s) {};
        auto blank = [](u8* sa, u8 status) {};
        this->_sp_callback = blank;
        this->_user_sp_callback = blank;
        this->_user_debug_cb = [](const char* s) { };
        this->_user_cmmc_sp_success_callback = cmmc_err_blank;
        this->_user_cmmc_sp_error_callback = cmmc_err_blank;
      }

      void begin(CMMC_SimplePair_mode_t mode, u8* pairkey = NULL, u8* message = NULL);
      void begin(CMMC_SimplePair_mode_t mode, cmmc_simple_pair_status_cb_t cb) {
        this->begin(mode, NULL, NULL, cb);
      };
      void begin(CMMC_SimplePair_mode_t mode, u8* pairkey, u8* message, cmmc_simple_pair_status_cb_t cb);
      void start();
      void mode(CMMC_SimplePair_mode_t);
      void set_pair_key(u8 *key);
      void set_pair_key(u8 b);
      void set_message(u8 *, int len = 16);
      void add_listener(simple_pair_status_cb_t);
      void debug(cmmc_debug_cb_t);
      void on(CMMC_SimplePair_event_t evt, cmmc_simple_pair_status_cb_t cb);
      int mode();
  private:
      char _debug_buffer[CSP_DEBUG_BUFFER];
      u8 _pair_key[16] = {0xff, 0x63, 0x6d, 0x6d, 0x63, 0x04, 0x05, 0x06, 0x07,
     0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0fa};
      u8 _message[16] = {0x00};
      CMMC_SimplePair_mode_t _mode;
      simple_pair_status_cb_t _sp_callback = NULL;
      simple_pair_status_cb_t _user_sp_callback = NULL;
      cmmc_debug_cb_t _user_debug_cb;
      cmmc_simple_pair_status_cb_t _user_cmmc_sp_error_callback = NULL;
      cmmc_simple_pair_status_cb_t _user_cmmc_sp_success_callback = NULL;
      void on_sp_st_finish(u8*);
      void on_sp_st_ap_recv_neg(u8*);
      void on_sp_st_wait_timeout(u8*);
      void on_sp_st_send_error(u8*);
      void on_sp_st_key_install_err(u8*);
      void on_sp_st_key_overlap_err(u8*);
      void on_sp_st_op_error(u8*);
      void on_sp_st_unknown_error(u8*);
      void on_sp_st_max(u8*);
      void _simple_pair_init();
      void debug_cb(const char*);
      void _preconfig_wifi_status(CMMC_SimplePair_mode_t mode);

};
#endif //CMMC_SimplePair_H
