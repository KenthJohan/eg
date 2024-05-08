#include "progress_motor.hpp"

void motor_driver(Adafruit_DS3502 dpot[4], int8_t m[4])
{
  digitalWrite(MOTOR_L0_PIN, m[MOTOR_L0] > 0);
  digitalWrite(MOTOR_L1_PIN, m[MOTOR_L1] > 0);
  digitalWrite(MOTOR_R0_PIN, m[MOTOR_R0] > 0);
  digitalWrite(MOTOR_R1_PIN, m[MOTOR_R1] > 0);
  dpot[MOTOR_L0].setWiper(abs(m[MOTOR_L0]));
  dpot[MOTOR_L1].setWiper(abs(m[MOTOR_L1]));
  dpot[MOTOR_R0].setWiper(abs(m[MOTOR_R0]));
  dpot[MOTOR_R1].setWiper(abs(m[MOTOR_R1]));
  Serial.printf("setWiper %03i %03i %03i %03i\n", m[MOTOR_L0], m[MOTOR_L1], m[MOTOR_R0], m[MOTOR_R1]);
}



void progress_motor(app_t * app, Adafruit_DS3502 dpot[MOTOR_COUNT]) {
  motor_driver(dpot, app->motors);
}