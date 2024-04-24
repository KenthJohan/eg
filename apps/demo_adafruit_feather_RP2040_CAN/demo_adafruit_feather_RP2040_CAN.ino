/*
 * Adafruit MCP2515 FeatherWing CAN Receiver Example
 */

#include <Adafruit_MCP2515.h>
#include <Adafruit_DS3502.h>
#include "config.h"
#include "progress.hpp"

Adafruit_DS3502 ds3502[4];
Adafruit_MCP2515 mcp(PIN_CAN_CS);


void Adafruit_DS3502_begin(SerialUSB& s, Adafruit_DS3502 &ds, uint8_t a) {
  s.printf("[begin] DS3502 %02X\n", a);
  if (!ds.begin(DS3502_I2CADDR_DEFAULT + a)) {
    Serial.printf("[error] DS3502 %02X\n", a);
    while(1) {
      delay(10);
    } 
  }
  s.printf("[found] DS3502 %02X\n", a);
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
  Adafruit_DS3502_begin(Serial, ds3502[0], 0x00);
  Adafruit_DS3502_begin(Serial, ds3502[1], 0x01);
  Adafruit_DS3502_begin(Serial, ds3502[2], 0x02);
  Adafruit_DS3502_begin(Serial, ds3502[3], 0x03);
  Adafruit_MCP2515_begin(Serial, mcp);

  Serial.println("[cansend] begin");
  mcp.beginPacket(CANID_BEGIN);
  mcp.write('h');
  mcp.write('e');
  mcp.write('l');
  mcp.write('l');
  mcp.write('o');
  mcp.endPacket();
  Serial.println("[cansend] end");
}

static int inc = 0;
static int inc2 = 0;

void loop() {
  progress(mcp, ds3502);
  inc++;
  if((inc % 10000) == 0) {
    mcp.beginPacket(CANID_MCU_TIME);
    mcp.write(inc2++);
    mcp.endPacket();
  }
}