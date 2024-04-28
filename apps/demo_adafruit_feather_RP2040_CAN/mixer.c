#include "mixer.h"
#include "config.h"
#include <stdlib.h>


void mixer(int32_t f, int32_t r, int32_t res, int8_t m[4])
{
  int32_t t[4];
  f -= abs(r);
  t[MOTOR_L0] = f + r;
  t[MOTOR_L1] = f + r;
  t[MOTOR_R0] = f - r;
  t[MOTOR_R1] = f - r;
  t[MOTOR_L0] *= 127;
  t[MOTOR_L1] *= 127;
  t[MOTOR_R0] *= 127;
  t[MOTOR_R1] *= 127;
  t[MOTOR_L0] /= res;
  t[MOTOR_L1] /= res;
  t[MOTOR_R0] /= res;
  t[MOTOR_R1] /= res;
  m[MOTOR_L0] = t[MOTOR_L0];
  m[MOTOR_L1] = t[MOTOR_L1];
  m[MOTOR_R0] = t[MOTOR_R0];
  m[MOTOR_R1] = t[MOTOR_R1];
}