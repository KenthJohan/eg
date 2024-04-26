#include "ibus.hpp"

void setup() 
{
  Serial.begin(115200);
  IBus.begin(Serial1);
}

void loop() 
{
  IBus.loop();
  //Serial.println(IBus.readChannel(0), HEX);
}

