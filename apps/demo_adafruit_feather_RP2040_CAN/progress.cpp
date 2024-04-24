#include "progress.hpp"
#include <Adafruit_MCP2515.h>
#include <Adafruit_DS3502.h>
#include "config.h"

void progress_can_rx(Adafruit_MCP2515 &can, uint8_t rx[], int len, Adafruit_DS3502 pot[4])
{
  switch(can.packetId()) {
    case CANID_DIGIPOTS:
      if(len >= 4) {
        Serial.printf("setWiper %03i %03i %03i %03i\n", rx[0], rx[1], rx[2], rx[3]);
        pot[0].setWiper(rx[0]);
        pot[1].setWiper(rx[1]);
        pot[2].setWiper(rx[2]);
        pot[3].setWiper(rx[3]);
      } else {
        Serial.printf("[warning] CANID_DIGIPOTS packet not supported\n");
      }
      break;

    case CANID_MOVE:
      if(len == 2) {
        uint8_t w[4];
        w[0] = rx[0] + rx[1];
        w[1] = rx[0];
        w[2] = rx[0] + rx[1];
        w[3] = rx[0];
        Serial.printf("setWiper %03i %03i %03i %03i\n", w[0], w[1], w[2], w[3]);
        pot[0].setWiper(w[0]);
        pot[1].setWiper(w[1]);
        pot[2].setWiper(w[2]);
        pot[3].setWiper(w[3]);
        can.beginPacket(CANID_DIGIPOTS);
        can.write(w[0]);
        can.write(w[1]);
        can.write(w[2]);
        can.write(w[3]);
        can.write(1);
        can.endPacket();
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
  if (len) {
    // received a packet
    Serial.printf("canid:%03X\n", can.packetId());

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