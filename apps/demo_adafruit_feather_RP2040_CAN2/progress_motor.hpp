#pragma once

#include <Adafruit_DS3502.h>
#include "config.h"

void progress_motor(app_t * app, Adafruit_DS3502 dpot[MOTOR_COUNT]);