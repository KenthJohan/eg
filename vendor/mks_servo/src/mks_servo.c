#include "mks_servo/mks_servo.h"
#include <stdio.h>
/*


void speedModeRun(uint8_t slaveAddr,uint8_t dir,uint16_t speed,uint8_t acc)
{
  int i;
  uint16_t checkSum = 0;

  txBuffer[0] = 0xFA;       //frame header
  txBuffer[1] = slaveAddr;  //slave address
  txBuffer[2] = 0xF6;       //function code
  txBuffer[3] = (dir<<7) | ((speed>>8)&0x0F); //High 4 bits for direction and speed
  txBuffer[4] = speed&0x00FF;   //8 bits lower
  txBuffer[5] = acc;            //acceleration
  txBuffer[6] = getCheckSum(txBuffer,6);  //Calculate checksum

  Serial.write(txBuffer,7);
}
*/

uint8_t mks_checksum_can(uint8_t can_id, uint8_t buffer[], uint8_t size)
{
	uint16_t sum = can_id;
	for (int i = 0; i < size; ++i) {
		uint8_t b = buffer[i];
		sum += b; // Calculate accumulated value
	}
	return sum & 0xFF; // return checksum
}

int mks_speed_mode(uint8_t tx[5], uint8_t can_id, uint8_t dir, uint16_t speed, uint8_t acc)
{
	uint8_t t = ((speed >> 8) & 0x0F); // 0001 tttt 0000 0000 -> 0000 0000 0001 tttt -> xxxx xxxx 0000 tttt
	uint8_t speed1 = (dir << 7) | t;   // 0000 000d -> d000 tttt
	uint8_t speed2 = speed & 0x00FF;   // 0001 tttt ssss ssss -> 0000 0000 ssss ssss
	// uint16_t speed3 = speed1 & 0x0FFF; //0001 tttt 0000 0000 -> 0000 3333 3333 3333
	// speed2 = speed3 & 0x00FF;
	tx[0] = 0xF6;
	tx[1] = speed1; // d000 tttt
	tx[2] = speed2; // ssss ssss
	tx[3] = acc;    // acceleration
	tx[4] = mks_checksum_can(can_id, tx, 4);
	return 5;
}

int mks_save_speed(uint8_t tx[3], uint8_t can_id, uint8_t speed)
{
	tx[0] = 0xFF;
	tx[1] = speed;
	tx[2] = mks_checksum_can(can_id, tx, 2);
	return 3;
}

int mks_cmd(uint8_t tx[], uint8_t can_id, uint8_t cmd)
{
	tx[0] = cmd;
	tx[1] = mks_checksum_can(can_id, tx, 4);
	return 2;
}

int32_t mks_i32_parse(uint8_t const rx[])
{
	int32_t v = (int32_t)(((uint32_t)rx[0] << 24) | ((uint32_t)rx[1] << 16) | ((uint32_t)rx[2] << 8) | ((uint32_t)rx[3] << 0));
	return v;
}

void mks_parse(uint8_t can_id, uint8_t rx[])
{
	uint8_t sum = mks_checksum_can(can_id, rx, 7);
	if(sum != rx[7]) {
		fprintf(stderr, "Checksum mismatch\n");
		return;
	}
	switch (rx[0]) {
	case MKS_REG_ACC_ENC_VAL:
		printf("MKS_REG_ACC_ENC_VAL: %i\n", mks_i32_parse(rx + 3));
		break;

	default:
		break;
	}
}