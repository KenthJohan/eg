#pragma once



#define CANID_BEGIN 10
#define CANID_PING 11
#define CANID_DIGIPOTS 20
#define CANID_MOTORS 24
#define CANID_MOVE 30
#define CANID_RC 40
#define CANID_MCU_TIME 100

#define WIPER_VALUE_PIN A0

// Set CAN bus baud rate
//#define CAN_BAUDRATE (10000) // -s0
#define CAN_BAUDRATE (500000) // -s6

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
