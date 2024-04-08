#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Accumulating encoder value
#define MKS_REG_ACC_ENC_VAL 0x31



#define MKS_CMD_STOP_AND_SAVE_SPEED 0xC8
#define MKS_CMD_STOP_AND_CLEAR_SPEED 0xCA

int mks_cmd(uint8_t tx[], uint8_t can_id, uint8_t cmd);
int mks_speed_mode(uint8_t tx[5], uint8_t can_id, uint8_t dir, uint16_t speed, uint8_t acc);
int mks_save_speed(uint8_t tx[3], uint8_t can_id, uint8_t cmd);


void mks_parse(uint8_t can_id, uint8_t rx[]);

#ifdef __cplusplus
}
#endif