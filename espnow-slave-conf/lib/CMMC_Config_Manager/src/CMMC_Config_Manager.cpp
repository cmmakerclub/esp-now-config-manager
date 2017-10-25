#include "CMMC_Config_Manager.h"
#include "FS.h"

void CMMC_Config_Manager::init(const char* filename) {   
  strcpy(this->filename_c, filename);
  USER_DEBUG_PRINTF("initializing SPIFFS ...");
  SPIFFS.begin();
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    String fileName = dir.fileName();
    size_t fileSize = dir.fileSize();
    USER_DEBUG_PRINTF("FS File: %s, size: %s", fileName.c_str(), String(fileSize).c_str());
  }
}

void CMMC_Config_Manager::commit() {
  static CMMC_Config_Manager *_this = this;
  load_config([](JsonObject * root) {
    _this->configFile = SPIFFS.open(_this->filename_c, "w");
    for (Items::iterator it = _this->items.begin(); it != _this->items.end(); ++it) {
      root->set(it->first, it->second);
    }
    root->printTo(_this->configFile);
    _this->configFile.close();
  });
}

void CMMC_Config_Manager::add_field(const char* key, const char* value) {
  strcpy(this->_k, key);
  strcpy(this->_v, value);
  USER_DEBUG_PRINTF(">>>[add_field] with %s:%s", key, value);
  static CMMC_Config_Manager *_this = this;
  items[_k] = _v;
  // show content:
  for (Items::iterator it = items.begin(); it != items.end(); ++it) {
    USER_DEBUG_PRINTF(":::: %s->%s", it->first.c_str(), it->second.c_str());
  }

  // USER_DEBUG_PRINTF("millis() = %lu\r\n", millis());
  USER_DEBUG_PRINTF(">>>/[add_field]");
}

void CMMC_Config_Manager::load_config(cmmc_json_loaded_cb_t cb) {
  USER_DEBUG_PRINTF("[load_config] Loading Config..");
  _open_file();
  size_t size = configFile.size() + 1;
  std::unique_ptr<char[]> buf(new char[size + 1]);
  bzero(buf.get(), size + 1);
  configFile.readBytes(buf.get(), size);
  configFile.close();
  USER_DEBUG_PRINTF("[load_config] config content ->%s<-", buf.get());
  JsonObject& json = this->jsonBuffer.parseObject(buf.get());
  if (json.success()) {
    USER_DEBUG_PRINTF("[load_config] Parsing config success.");
    if (cb != NULL) {
      USER_DEBUG_PRINTF("[load_config] calling callback fn");
      cb(&json);
    }
  }
  else {
    USER_DEBUG_PRINTF("[load_config] Failed to parse config file.");
    _init_json_file(cb);
  }
}

void CMMC_Config_Manager::_init_json_file(cmmc_json_loaded_cb_t cb) {
  USER_DEBUG_PRINTF("[_init_json_file]");
  configFile = SPIFFS.open(this->filename_c, "w");
  JsonObject& json = this->jsonBuffer.createObject();
  json.printTo(configFile);
  USER_DEBUG_PRINTF("[_init_json_file] closing file..");
  configFile.close();
  load_config(cb);
}

void CMMC_Config_Manager::dump_json_object(cmmc_dump_cb_t printer) {
  this->load_config();
  // if (this->currentJsonObject == NULL) {
  //   return;
  // }
  // else {
  //   this->currentJsonObject->printTo(Serial);
  //   char str_buffer[30];
  //   JsonObject* obj = this->currentJsonObject;
  //   for (JsonObject::iterator it = obj->begin(); it != obj->end(); ++it) {
  //     const char* key = it->key;
  //     const char* value = it->value;
  //     sprintf(str_buffer, "[key] %s -> %s\r\n", key, value);
  //     printer(str_buffer, key, value);
  //   }
  // }
}

void CMMC_Config_Manager::add_debug_listener(cmmc_debug_cb_t cb) {
  if (cb != NULL) {
    this->_user_debug_cb = cb;
  }
}

void CMMC_Config_Manager::_open_file()  {
  USER_DEBUG_PRINTF("[open_file] open filename: %s", this->filename_c);
  if (SPIFFS.exists(this->filename_c)) {
    configFile = SPIFFS.open(this->filename_c, "r");
    USER_DEBUG_PRINTF("[open_file] config size = %lu bytes", configFile.size());
    if (configFile.size() > 512) {
      USER_DEBUG_PRINTF("[open_file] Config file size is too large");
    } else {
      USER_DEBUG_PRINTF("[open_file] check file size ok.");
    }
  } else { // file not exists
    USER_DEBUG_PRINTF("[open_file] file not existsing so create a new file");
    _init_json_file();
  }
}
