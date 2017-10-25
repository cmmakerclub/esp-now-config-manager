#include "CMMC_SimplePair.h"

void CMMC_SimplePair::debug_cb(const char* str) {
  this->_user_debug_cb(str);
}

void CMMC_SimplePair::mode(CMMC_SimplePair_mode_t mode) {
  this->_mode = mode;
}

void CMMC_SimplePair::set_pair_key(u8 key) {
  this->_pair_key[0] = key;
}

void CMMC_SimplePair::set_pair_key(u8 *key) {
    if (key == NULL) return;
    memcpy(this->_pair_key, key, 16);
}

void CMMC_SimplePair::set_message(u8 *msg, int len) {
    if (msg == NULL) return;
    memcpy(this->_message, msg, len);
}

void CMMC_SimplePair::on_sp_st_finish(u8* sa) {
  if (this->_mode == CSP_MODE_AP)
  {
    u8 ex_key[16];
    simple_pair_get_peer_ref(NULL, NULL, ex_key);
    this->debug_cb("Simple Pair: AP FINISH");
    _user_cmmc_sp_success_callback(SP_ST_AP_FINISH, sa, ex_key);
    /* if test ok , deinit simple pair */
    simple_pair_deinit();
  }
  else
  {
    u8 ex_key[16];
    simple_pair_get_peer_ref(NULL, NULL, ex_key);
    this->debug_cb("Simple Pair: STA FINISH");
    _user_cmmc_sp_success_callback(SP_ST_STA_FINISH, sa, ex_key);
    simple_pair_deinit();
  }
}

int CMMC_SimplePair::mode() {
  return this->_mode;
}

void CMMC_SimplePair::on(CMMC_SimplePair_event_t evt, cmmc_simple_pair_status_cb_t cb) {
  if (evt == CSP_EVENT_ERROR && cb != NULL) {
    this->_user_cmmc_sp_error_callback = cb;
  }
  else if (evt == CSP_EVENT_SUCCESS && cb != NULL) {
    this->_user_cmmc_sp_success_callback = cb;
  }
  else {
    // TODO: handle invalid type
  }
}

void CMMC_SimplePair::_simple_pair_init() {
  int ret;
  static CMMC_SimplePair *_this = this;

  if (this->_mode == CSP_MODE_AP) {
    wifi_set_opmode(SOFTAP_MODE);
    /* init simple pair */
    ret = simple_pair_init();
    if (ret) {
      sprintf(this->_debug_buffer, "Simple Pair: init error, %d", ret);
      debug_cb(this->_debug_buffer);
      return;
    }
    /* register simple pair status callback function */
    ret = register_simple_pair_status_cb(this->_sp_callback);
    if (ret) {
      sprintf(this->_debug_buffer, "Simple Pair: register status cb error, %d", ret);
      debug_cb(this->_debug_buffer);
      return;
    }
    else {
      this->debug_cb("Simple Pair: AP Enter Announce Mode ...");
      /* ap must enter announce mode , so the sta can know which ap is ready to simple pair */
      ret = simple_pair_ap_enter_announce_mode();
      if (ret) {
        sprintf(this->_debug_buffer, "Simple Pair: AP Enter Announce Mode Error, %d", ret);
        debug_cb(this->_debug_buffer);
        return;
      }
    }
  }
  else if (this->_mode == CSP_MODE_STA) {
    wifi_set_opmode(STATION_MODE);
    /* init simple pair */
    ret = simple_pair_init();
    if (ret) {
      sprintf(this->_debug_buffer, "Simple Pair: init error, %d", ret);
      debug_cb(this->_debug_buffer);
      return;
    }
    /* register simple pair status callback function */
    ret = register_simple_pair_status_cb(this->_sp_callback);
    if (ret) {
      sprintf(this->_debug_buffer, "Simple Pair: register status cb error, %d", ret);
      debug_cb(this->_debug_buffer);
      return;
    }
    this->debug_cb("Simple Pair: STA Enter Scan Mode ...");
    ret = simple_pair_sta_enter_scan_mode();
    if (ret) {
      sprintf(this->_debug_buffer, "Simple Pair: STA Enter Scan Mode Error, %d", ret);
      debug_cb(this->_debug_buffer);
      return;
    } else {
      /* scan ap to searh which ap is ready to simple pair */
      sprintf(this->_debug_buffer, "Simple Pair: STA Scan AP ...");
      debug_cb(this->_debug_buffer);
      wifi_station_scan(NULL, [](void *arg, STATUS status) {
        int ret;
        if (status == OK) {
          _this->debug_cb("scan_done");
          struct bss_info *bss_link = (struct bss_info *)arg;
          while (bss_link != NULL) {
            String ssid = String(reinterpret_cast<char*>(bss_link->ssid));
            // String bssid = String(reinterpret_cast<char*>(bss_link->bssid));
            String rssi = String(bss_link->rssi);
            String auth_mode = String(bss_link->authmode);

            sprintf(_this->_debug_buffer, "%s (%s, %s)", ssid.c_str(),
              rssi.c_str(), auth_mode.c_str());
            _this->debug_cb(_this->_debug_buffer);

            if (bss_link->simple_pair) {
              sprintf(_this->_debug_buffer,
                "Simple Pair: bssid %02x:%02x:%02x:%02x:%02x:%02x Ready!",
                bss_link->bssid[0], bss_link->bssid[1],
                bss_link->bssid[2], bss_link->bssid[3],
                bss_link->bssid[4], bss_link->bssid[5]);
              _this->debug_cb(_this->_debug_buffer);
              simple_pair_set_peer_ref(bss_link->bssid, _this->_pair_key, NULL);
              ret = simple_pair_sta_start_negotiate();
              if (ret) {
                _this->debug_cb("Simple Pair: STA start NEG Failed");
              }
              else {
                _this->debug_cb("Simple Pair: STA start NEG OK");
              }
              break;
            }
            // bss_link = bss_link->next->stqe_next;
            _this->debug_cb("next...");
            bss_link = bss_link->next;
          }
        } else {
          sprintf(_this->_debug_buffer, "err, scan status %d", status);
          _this->debug_cb(_this->_debug_buffer);
        }
      });
    }
  }
  else {
    // mode is neither master-mode or slave-mode
  }
}

void CMMC_SimplePair::on_sp_st_ap_recv_neg(u8* sa) {
  /* AP recv a STA's negotiate request */
  this->debug_cb("Simple Pair: Recv STA Negotiate Request");
  simple_pair_set_peer_ref(sa, this->_pair_key, this->_message);
  // check for negotiate
  simple_pair_ap_start_negotiate();
  // simple_pair_ap_refuse_negotiate();
}

void CMMC_SimplePair::on_sp_st_wait_timeout(u8* sa) { }
void CMMC_SimplePair::on_sp_st_send_error(u8* sa) { }
void CMMC_SimplePair::on_sp_st_key_install_err(u8* sa) { }
void CMMC_SimplePair::on_sp_st_key_overlap_err(u8* sa) { }
void CMMC_SimplePair::on_sp_st_op_error(u8* sa) { }
void CMMC_SimplePair::on_sp_st_unknown_error(u8* sa) { }
void CMMC_SimplePair::on_sp_st_max(u8* sa) { }

void CMMC_SimplePair::debug(cmmc_debug_cb_t cb) {
  if (cb != NULL) {
    this->_user_debug_cb = cb;
  }
}
void CMMC_SimplePair:: _preconfig_wifi_status(CMMC_SimplePair_mode_t mode) {
    if (mode == CSP_MODE_AP) {
      WiFi.disconnect(0);
      WiFi.mode(WIFI_AP);
      String apName =  String("CMMC-") + String(ESP.getChipId());
      WiFi.softAP(apName.c_str());
    }
    else {
      WiFi.disconnect(0);
      WiFi.mode(WIFI_STA);
    }
}


void CMMC_SimplePair::begin(CMMC_SimplePair_mode_t mode, u8 *pairkey, u8 *msg,
  cmmc_simple_pair_status_cb_t cb) {
    this->on(CSP_EVENT_SUCCESS, cb);
    this->on(CSP_EVENT_ERROR, cb);
    this->begin(mode, pairkey, msg);
}
void CMMC_SimplePair::begin(CMMC_SimplePair_mode_t mode, u8 *pairkey, u8 *msg) {

    static CMMC_SimplePair* _this = this;

    this->mode(mode);
    this->set_message(msg);
    this->set_pair_key(pairkey);

    this->_sp_callback = [](u8 *sa, u8 status) {
        sprintf(_this->_debug_buffer, "event %d", status);
        _this->debug_cb(_this->_debug_buffer);
        _this->_user_sp_callback(sa, status);
        switch (status) {
          case 0:
            _this->on_sp_st_finish(sa);
            break;
          case SP_ST_AP_RECV_NEG:
            _this->on_sp_st_ap_recv_neg(sa);
            break;
          case SP_ST_WAIT_TIMEOUT:
            _this->on_sp_st_wait_timeout(sa);
            _this->_user_cmmc_sp_error_callback(status, sa, (const u8*)"SP_ST_WAIT_TIMEOUT");
            break;
          case SP_ST_SEND_ERROR:
            _this->on_sp_st_send_error(sa);
            _this->_user_cmmc_sp_error_callback(status, sa, (const u8*)"SP_ST_SEND_ERROR");
            break;
          case SP_ST_KEY_INSTALL_ERR:
            _this->on_sp_st_key_install_err(sa);
            _this->_user_cmmc_sp_error_callback(status, sa, (const u8*)"SP_ST_KEY_INSTALL_ERR");
            break;
          case SP_ST_KEY_OVERLAP_ERR:
            _this->on_sp_st_key_overlap_err(sa);
            _this->_user_cmmc_sp_error_callback(status, sa, (const u8*)"SP_ST_KEY_OVERLAP_ERR");
            break;
          case SP_ST_OP_ERROR:
            _this->on_sp_st_op_error(sa);
            _this->_user_cmmc_sp_error_callback(status, sa, (const u8*)"SP_ST_OP_ERROR");
            break;
          case SP_ST_UNKNOWN_ERROR:
            _this->on_sp_st_unknown_error(sa);
            _this->_user_cmmc_sp_error_callback(status, sa, (const u8*)"SP_ST_UNKNOWN_ERROR");
            break;
          case SP_ST_MAX:
            _this->on_sp_st_max(sa);
            _this->_user_cmmc_sp_error_callback(status, sa, (const u8*)"SP_ST_MAX");
            break;
          default:
            _this->on_sp_st_unknown_error(sa);
            _this->_user_cmmc_sp_error_callback(status, sa, (const u8*)"SP_ST_UNKNOWN_ERROR");
              break;
          }
    };
}

void CMMC_SimplePair::add_listener(simple_pair_status_cb_t cb) {
  this->_user_sp_callback = cb;
}

void CMMC_SimplePair::start() {
    _preconfig_wifi_status(this->_mode);
    this->_simple_pair_init();
}
