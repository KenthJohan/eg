#include "progress_can.hpp"
#include "mixer.h"

void progress_can_rx(app_t * app, Adafruit_MCP2515 &can, int id, uint8_t rx[], int len)
{
  switch(id) {
    case CANID_MOTORS:
      if(len >= 4) {
        app->motors[MOTOR_L0] = rx[0];
        app->motors[MOTOR_L1] = rx[1];
        app->motors[MOTOR_R0] = rx[2];
        app->motors[MOTOR_R1] = rx[3];
        app->mix_mode = MIX_MODE_4;
      } else {
        Serial.printf("[warning] CANID_DIGIPOTS packet not supported\n");
      }
      break;

    case CANID_MOVE:
      if(len == 2) {
        app->move[0] = rx[0];
        app->move[1] = rx[1];
        app->mix_mode = MIX_MODE_MOVE;
      } else {
        Serial.printf("[warning] CANID_DIGIPOTS_WIPER packet not supported\n");
      }
      break;

    case CANID_PING:
      app->ping_can = true;
      break;
  }
}

#define RX_BUF_CAP 128
void progress_can(app_t * app, Adafruit_MCP2515 &can)
{
  // try to parse packet
  int len = can.parsePacket();
  //Serial.printf("len:%03i\n", len);
  if (len) {
    // received a packet
    //Serial.printf("canid:%03i\n", can.packetId());

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
    progress_can_rx(app, can, can.packetId(), rx, len);
  }

  can.beginPacket(CANID_MOTORS);
  can.write(app->motors[MOTOR_L0]);
  can.write(app->motors[MOTOR_L1]);
  can.write(app->motors[MOTOR_R0]);
  can.write(app->motors[MOTOR_R1]);
  can.write(1);
  can.endPacket();

  can.beginPacket(CANID_MOVE);
  can.write(app->move[0]);
  can.write(app->move[1]);
  can.write(1);
  can.endPacket();

  can.beginPacket(CANID_RC_01234);
  can.write(app->rcvals[0]);
  can.write(app->rcvals[1]);
  can.write(app->rcvals[2]);
  can.write(app->rcvals[3]);
  can.write(app->rcvals[4]);
  can.endPacket();

  can.beginPacket(CANID_RC_56789);
  can.write(app->rcvals[5]);
  can.write(app->rcvals[6]);
  can.write(app->rcvals[7]);
  can.write(app->rcvals[8]);
  can.write(app->rcvals[9]);
  can.endPacket();

  if (app->ping_can) {
    can.beginPacket(CANID_PING);
    can.write('h');
    can.write('e');
    can.write('l');
    can.write('l');
    can.write('o');
    can.endPacket();
    app->ping_can = false;
  }


}