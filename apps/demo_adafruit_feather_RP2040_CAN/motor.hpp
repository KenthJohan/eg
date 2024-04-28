#pragma once

#include <Adafruit_MCP2515.h>
#include <Adafruit_DS3502.h>
#include "config.h"

void motor_driver(Adafruit_MCP2515 &can, Adafruit_DS3502 dpot[4], int8_t m[4]);