#include "motor.hpp"


void motor_driver(Adafruit_MCP2515 &can, Adafruit_DS3502 dpot[4], int8_t m[4])
{
  digitalWrite(MOTOR_L0_PIN, m[MOTOR_L0] > 0);
  digitalWrite(MOTOR_L1_PIN, m[MOTOR_L1] > 0);
  digitalWrite(MOTOR_R0_PIN, m[MOTOR_R0] > 0);
  digitalWrite(MOTOR_R1_PIN, m[MOTOR_R1] > 0);
  dpot[MOTOR_L0].setWiper(abs(m[MOTOR_L0]));
  dpot[MOTOR_L1].setWiper(abs(m[MOTOR_L1]));
  dpot[MOTOR_R0].setWiper(abs(m[MOTOR_R0]));
  dpot[MOTOR_R1].setWiper(abs(m[MOTOR_R1]));
  can.beginPacket(CANID_MOTORS);
  can.write(m[MOTOR_L0]);
  can.write(m[MOTOR_L1]);
  can.write(m[MOTOR_R0]);
  can.write(m[MOTOR_R1]);
  can.write(1);
  can.endPacket();
  Serial.printf("setWiper %03i %03i %03i %03i\n", m[MOTOR_L0], m[MOTOR_L1], m[MOTOR_R0], m[MOTOR_R1]);
}
