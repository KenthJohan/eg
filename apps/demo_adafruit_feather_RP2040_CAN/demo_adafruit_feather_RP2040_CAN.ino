/*
 * Adafruit MCP2515 FeatherWing CAN Receiver Example
 */

#include <Adafruit_MCP2515.h>
#include <Adafruit_DS3502.h>
#include "config.h"
#include "progress.hpp"

Adafruit_DS3502 ds3502[4];
Adafruit_MCP2515 mcp(PIN_CAN_CS);


void Adafruit_DS3502_begin(SerialUSB& s, Adafruit_DS3502 &ds) {
  s.println("[begin] DS3502");
  if (!ds.begin(DS3502_I2CADDR_DEFAULT)) {
    Serial.println("[error] DS3502");
    while(1) {
      delay(10);
    } 
  }
  s.println("[found] DS3502");
}


void Adafruit_MCP2515_begin(SerialUSB& s, Adafruit_MCP2515 &m) {
  s.println("[begin] MCP2515");
  if (!m.begin(CAN_BAUDRATE)) {
    s.println("[error] MCP2515");
    while(1) {
      delay(10);
    } 
  }
  s.println("[found] MCP2515");
}

void setup() {
  Serial.begin(115200);
  while(!Serial) {
    delay(10);
  }
  Adafruit_DS3502_begin(Serial, ds3502[0]);
  Adafruit_MCP2515_begin(Serial, mcp);
}


void loop() {
  progress(mcp, ds3502);
}