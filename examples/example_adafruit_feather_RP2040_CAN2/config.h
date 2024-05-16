#pragma once
#include <stdint.h>
#include <stdbool.h>


#define CANID_BEGIN 10
#define CANID_PING 11
#define CANID_MOTORS 24
#define CANID_MOVE 30
#define CANID_RC_01234 40
#define CANID_RC_56789 41
#define CANID_MCU_TIME 100

#define WIPER_VALUE_PIN A0

typedef enum {
  MOTOR_L0,
  MOTOR_L1,
  MOTOR_R0,
  MOTOR_R1,
  MOTOR_COUNT
} motor_index_t;


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

typedef enum {
  RC_INDEX_0,
  RC_INDEX_1,
  RC_INDEX_2,
  RC_INDEX_3,
  RC_INDEX_4,
  RC_INDEX_5,
  RC_INDEX_6,
  RC_INDEX_7,
  RC_INDEX_8,
  RC_INDEX_9,
  RC_INDEX_COUNT,
} rc_index_t;


typedef enum {
  MIX_MODE_4,
  MIX_MODE_MOVE,
  MIX_MODE_COUNT
} mix_mode_t;


typedef struct {
  int8_t move[2];
  int8_t motors[MOTOR_COUNT];
  int32_t rcvals[RC_INDEX_COUNT];
  mix_mode_t mix_mode;
  bool ping_can;
} app_t;


#ifdef __cplusplus
}
#endif
