#include "progress.hpp"
#include <Adafruit_MCP2515.h>
#include <Adafruit_DS3502.h>
#include "config.h"
#include "motor.hpp"
#include "mixer.h"

void progress_can_rx(Adafruit_MCP2515 &can, uint8_t rx[], int len, Adafruit_DS3502 dpot[4])
{
  switch(can.packetId()) {
    case CANID_MOTORS:
      if(len >= 4) {
        int8_t m[4];
        m[MOTOR_L0] = rx[0];
        m[MOTOR_L1] = rx[1];
        m[MOTOR_R0] = rx[2];
        m[MOTOR_R1] = rx[3];
        motor_driver(can, dpot, m);
      } else {
        Serial.printf("[warning] CANID_DIGIPOTS packet not supported\n");
      }
      break;

    case CANID_MOVE:
      if(len == 2) {
        int32_t f = (int8_t)rx[0];
        int32_t r = (int8_t)rx[1];
        int32_t t[4];
        int8_t m[4];
        mixer(f, r, 127, t, m);
        Serial.printf("setWiper %03i %03i %03i %03i\n", t[MOTOR_L0], t[MOTOR_L1], t[MOTOR_R0], t[MOTOR_R1]);
        motor_driver(can, dpot, m);
      } else {
        Serial.printf("[warning] CANID_DIGIPOTS_WIPER packet not supported\n");
      }
      break;

    case CANID_PING:
      can.beginPacket(CANID_PING);
      can.write('h');
      can.write('e');
      can.write('l');
      can.write('l');
      can.write('o');
      can.endPacket();
      break;
  }
}

#define RX_BUF_CAP 128
void progress(Adafruit_MCP2515 &can, Adafruit_DS3502 pot[4])
{
  // try to parse packet
  int len = can.parsePacket();
  //Serial.printf("len:%03i\n", len);
  if (len) {
    // received a packet
    Serial.printf("canid:%03i\n", can.packetId());

    if (can.packetExtended()) {
      Serial.print("packetExtended(): ");
      Serial.println(can.packetDlc());
      return;
    }

    // Serial.print("packet with id 0x");
    // Serial.print(can.packetId(), HEX);
    if (can.packetRtr()) {
      Serial.print("Remote transmission request: ");
      Serial.println(can.packetDlc());
      return;
    } 


    // Copy rx buffer from crappy OOP class to rx:
    // Why can't we acces the rx buffer from the crappy OOP class?
    uint8_t rx[RX_BUF_CAP];
    int i = 0;
    while (can.available() && (i < RX_BUF_CAP)) {
      //Serial.printf("%02X", (char)can.read());
      rx[i] = (char)can.read();
      i++;
    }
    progress_can_rx(can, rx, len, pot);
    



  }
}