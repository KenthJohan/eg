/*
 * Adafruit MCP2515 FeatherWing CAN Receiver Example
 */

#include <Adafruit_MCP2515.h>

Adafruit_MCP2515 mcp(PIN_CAN_CS);
#define CAN_BAUDRATE0 (10000) // -s0
#define CAN_BAUDRATE6 (500000) // -s6
#define CANID_SPAM_START 0x101
#define CANID_SPAM_LOOP 0x102


void Adafruit_MCP2515_begin(SerialUSB& s, Adafruit_MCP2515 &m) {
  s.println("[begin] MCP2515");
  if (!m.begin(CAN_BAUDRATE6)) {
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
  Adafruit_MCP2515_begin(Serial, mcp);

  Serial.println("[cansend] begin");
  mcp.beginPacket(CANID_SPAM_START);
  mcp.write('h');
  mcp.write('e');
  mcp.write('l');
  mcp.write('l');
  mcp.write('o');
  mcp.endPacket();
  Serial.println("[cansend] end");
}

static uint8_t inc = 0;
void loop() {
  mcp.beginPacket(CANID_SPAM_LOOP);
  mcp.write(inc++);
  mcp.endPacket();
  delay(100);
}