#pragma once



#define CANID_BEGIN 10
#define CANID_PING 11
#define CANID_MOTORS 24
#define CANID_MOVE 30
#define CANID_RC_01234 40
#define CANID_RC_56789 41
#define CANID_MCU_TIME 100

#define WIPER_VALUE_PIN A0

#define MOTOR_L0 0
#define MOTOR_L1 1
#define MOTOR_R0 2
#define MOTOR_R1 3

#define MOTOR_L0_PIN D12
#define MOTOR_L1_PIN D11
#define MOTOR_R0_PIN D10
#define MOTOR_R1_PIN D9



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
