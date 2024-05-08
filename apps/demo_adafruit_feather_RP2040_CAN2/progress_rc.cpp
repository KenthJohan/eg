#include "progress_rc.hpp"
#include "ibus.hpp"

void progress_rc(app_t * app)
{
  app->rcvals[0] = ((int32_t)IBus.readChannel(0) - 1500) * 255 / 1000;
  app->rcvals[1] = ((int32_t)IBus.readChannel(1) - 1500) * 255 / 1000;
  app->rcvals[2] = ((int32_t)IBus.readChannel(2) - 1500) * 255 / 1000;
  app->rcvals[3] = ((int32_t)IBus.readChannel(3) - 1500) * 255 / 1000;
  app->rcvals[4] = ((int32_t)IBus.readChannel(4) - 1500) * 255 / 1000;
  app->rcvals[5] = ((int32_t)IBus.readChannel(5) - 1500) * 255 / 1000;
  app->rcvals[6] = ((int32_t)IBus.readChannel(6) - 1500) * 255 / 1000;
  app->rcvals[7] = ((int32_t)IBus.readChannel(7) - 1500) * 255 / 1000;
  app->rcvals[8] = ((int32_t)IBus.readChannel(8) - 1500) * 255 / 1000;
  app->rcvals[9] = ((int32_t)IBus.readChannel(9) - 1500) * 255 / 1000;


/*
  Serial.printf("%03i ", app->rcvals[0]);
  Serial.printf("%03i ", app->rcvals[1]);
  Serial.printf("%03i ", app->rcvals[2]);
  Serial.printf("%03i ", app->rcvals[3]);
  Serial.printf("%03i ", app->rcvals[4]);
  Serial.printf("%03i ", app->rcvals[5]);
  Serial.printf("%03i ", app->rcvals[6]);
  Serial.printf("%03i ", app->rcvals[7]);
  Serial.printf("%03i ", app->rcvals[8]);
  Serial.printf("%03i ", app->rcvals[9]);
  Serial.printf("\n");
  */

}