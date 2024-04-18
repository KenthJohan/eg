#pragma once



#define CANID_PING 0x11
#define CANID_DIGIPOTS_WIPER 0x55
#define CANID_DIGIPOTS_READY 0x56

#define WIPER_VALUE_PIN A0

// Set CAN bus baud rate
#define CAN_BAUDRATE (500000)

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
  CMD_SEND_PING,
  CMD_TRY_CONNECT_CAN,
  CMD_TRY_CONNECT_DPOT0,
  CMD_TRY_CONNECT_DPOT1,
  CMD_TRY_CONNECT_DPOT2,
  CMD_TRY_CONNECT_DPOT3
} cmd_t;




#ifdef __cplusplus
}
#endif