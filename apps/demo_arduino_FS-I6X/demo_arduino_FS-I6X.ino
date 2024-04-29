#include "ibus.hpp"

void setup() 
{
  Serial.begin(115200);
  IBus.begin(Serial1);
}

void loop() 
{
  IBus.loop();
  Serial.printf("%03i ", IBus.readChannel(0));
  Serial.printf("%03i ", IBus.readChannel(1));
  Serial.printf("%03i ", IBus.readChannel(2));
  Serial.printf("%03i ", IBus.readChannel(3));
  Serial.printf("%03i ", IBus.readChannel(4));
  Serial.printf("%03i ", IBus.readChannel(5));
  Serial.printf("%03i ", IBus.readChannel(6));
  Serial.printf("%03i ", IBus.readChannel(7));
  Serial.printf("%03i ", IBus.readChannel(8));
  Serial.printf("%03i\n", IBus.readChannel(9));
}

