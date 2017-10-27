#ifndef CMMC_RX_Parser_H
#define CMMC_RX_Parser_H

#include <Arduino.h>

#define DATA_BUFFER 	128

typedef enum {
	WAIT_STATE = 0,
	CMD_STATE,
	DATA_STATE
} CMMC_SERIAL_CMD_STATE;


#define CMMC_SLEEP_TIME_CMD 0x88

typedef struct __attribute((__packed__)) {
	uint8_t  cmd;
  uint16_t len;
	uint8_t  data[DATA_BUFFER];
} CMMC_SERIAL_PACKET_T;

typedef void (*cmmc_void_cb_t)(CMMC_SERIAL_PACKET_T *packet);


class CMMC_RX_Parser
{
    public:
      // constructor
      CMMC_RX_Parser(Stream *s) { 
      	this->_serial = s;
      }
      ~CMMC_RX_Parser() {}

      void init();
      void _parse(uint8_t data);
      void process();
      void on_command_arrived(cmmc_void_cb_t cb) {
        this->_user_on_data = cb;
      }

    private:
    	Stream *_serial;
    	CMMC_SERIAL_CMD_STATE _state;
    	CMMC_SERIAL_PACKET_T _packet;
      CMMC_SERIAL_PACKET_T _user_packet;
      cmmc_void_cb_t _user_on_data;
};

#endif //CMMC_RX_Parser_H
