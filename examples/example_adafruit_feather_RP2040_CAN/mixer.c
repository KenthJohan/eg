#include "mixer.h"
#include "config.h"
#include <stdlib.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define CLAMP(x,lo,hi) MIN(hi,MAX(lo,x))

void mixer(int32_t f, int32_t r, int32_t res, int32_t t[4], int8_t m[4])
{
  t[MOTOR_L0] = f + r;
  t[MOTOR_L1] = f + r;
  t[MOTOR_R0] = f - r;
  t[MOTOR_R1] = f - r;
  t[MOTOR_L0] = CLAMP(t[MOTOR_L0], -127, 127);
  t[MOTOR_L1] = CLAMP(t[MOTOR_L1], -127, 127);
  t[MOTOR_R0] = CLAMP(t[MOTOR_R0], -127, 127);
  t[MOTOR_R1] = CLAMP(t[MOTOR_R1], -127, 127);
  m[MOTOR_L0] = t[MOTOR_L0];
  m[MOTOR_L1] = t[MOTOR_L1];
  m[MOTOR_R0] = t[MOTOR_R0];
  m[MOTOR_R1] = t[MOTOR_R1];
}