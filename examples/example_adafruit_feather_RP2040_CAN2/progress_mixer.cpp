#include "progress_mixer.hpp"


#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define CLAMP(x,lo,hi) MIN(hi,MAX(lo,x))

void dummy_mixer(int32_t f, int32_t r, int32_t res, int8_t m[MOTOR_COUNT])
{
  int32_t t[MOTOR_COUNT];
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

void progress_mixer(app_t * app)
{
  // Contol motors from RC handcontroller when a button is switched:
  if (app->rcvals[9] == 127) {
    app->move[0] = (int8_t)app->rcvals[1];
    app->move[1] = (int8_t)app->rcvals[0];
    app->mix_mode = MIX_MODE_MOVE;
  } 

  switch(app->mix_mode) {
    case MIX_MODE_4:
      break;
    case MIX_MODE_MOVE:
      dummy_mixer(app->move[0], app->move[1], 127, app->motors);
      break;
  }
}