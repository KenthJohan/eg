/*
 * Adafruit MCP2515 FeatherWing CAN Receiver Example
 */

#include <Adafruit_MCP2515.h>
#include <Adafruit_DS3502.h>
#include "config.h"
#include "progress.hpp"
#include "ibus.hpp"
#include "motor.hpp"
#include "mixer.h"
#include <hardware/watchdog.h>

Adafruit_DS3502 ds3502[4];
Adafruit_MCP2515 mcp(PIN_CAN_CS);


void Adafruit_DS3502_begin(SerialUSB& s, Adafruit_DS3502 &ds, uint8_t a) {
  s.printf("[begin] DS3502 %02X\n", a);
  if (!ds.begin(DS3502_I2CADDR_DEFAULT + a)) {
    Serial.printf("[error] DS3502 %02X\n", a);
    while(0) {
      delay(100);
    } 
  }
  s.printf("[found] DS3502 %02X\n", a);
}


void Adafruit_MCP2515_begin(SerialUSB& s, Adafruit_MCP2515 &m) {
  s.println("[begin] MCP2515");
  if (!m.begin(CAN_BAUDRATE)) {
    s.println("[error] MCP2515");
    while(0) {
      delay(10);
    }
  }
  s.println("[found] MCP2515");
}


static uint8_t mcu_time0 = 0;
bool repeating_timer_callback(struct repeating_timer *t) {
    mcp.beginPacket(CANID_MCU_TIME);
    mcp.write(mcu_time0++);
    mcp.endPacket();
    return true;
}

struct repeating_timer timer;


void setup() {
  bool watchrebooted = watchdog_caused_reboot();

  Serial.begin(115200);
  for(int i = 0; i < 100; ++i){
    if(Serial) {
      break;
    }
    delay(10);
  }

  Serial.printf("[watchrebooted] %i\n", watchrebooted);

  watchdog_enable((0x7fffff / 8) / 10, false);
  watchdog_start_tick(12);
  add_repeating_timer_ms(500, repeating_timer_callback, NULL, &timer);
  Adafruit_DS3502_begin(Serial, ds3502[0], 0x00);
  Adafruit_DS3502_begin(Serial, ds3502[1], 0x01);
  Adafruit_DS3502_begin(Serial, ds3502[2], 0x02);
  Adafruit_DS3502_begin(Serial, ds3502[3], 0x03);
  Adafruit_MCP2515_begin(Serial, mcp);
  pinMode(MOTOR_L0_PIN, OUTPUT);
  pinMode(MOTOR_L1_PIN, OUTPUT);
  pinMode(MOTOR_R0_PIN, OUTPUT);
  pinMode(MOTOR_R1_PIN, OUTPUT);
  digitalWrite(MOTOR_L0_PIN, 0);
  digitalWrite(MOTOR_L1_PIN, 0);
  digitalWrite(MOTOR_R0_PIN, 0);
  digitalWrite(MOTOR_R1_PIN, 0);

  Serial.println("[cansend] begin");
  mcp.beginPacket(CANID_BEGIN);
  mcp.write('h');
  mcp.write('e');
  mcp.write('l');
  mcp.write('l');
  mcp.write('o');
  mcp.endPacket();
  Serial.println("[cansend] end");

  IBus.begin(Serial1);
}



int32_t rcval[10];

void loop() {
  IBus.loop();
  progress(mcp, ds3502);


  rcval[0] = ((int32_t)IBus.readChannel(0) - 1500) * 255 / 1000;
  rcval[1] = ((int32_t)IBus.readChannel(1) - 1500) * 255 / 1000;
  rcval[2] = ((int32_t)IBus.readChannel(2) - 1500) * 255 / 1000;
  rcval[3] = ((int32_t)IBus.readChannel(3) - 1500) * 255 / 1000;
  rcval[4] = ((int32_t)IBus.readChannel(4) - 1500) * 255 / 1000;
  rcval[5] = ((int32_t)IBus.readChannel(5) - 1500) * 255 / 1000;
  rcval[6] = ((int32_t)IBus.readChannel(6) - 1500) * 255 / 1000;
  rcval[7] = ((int32_t)IBus.readChannel(7) - 1500) * 255 / 1000;
  rcval[8] = ((int32_t)IBus.readChannel(8) - 1500) * 255 / 1000;
  rcval[9] = ((int32_t)IBus.readChannel(9) - 1500) * 255 / 1000;

  if (rcval[8] < 0) {
    watchdog_update();
  }

  mcp.beginPacket(CANID_RC_01234);
  mcp.write(rcval[0]);
  mcp.write(rcval[1]);
  mcp.write(rcval[2]);
  mcp.write(rcval[3]);
  mcp.write(rcval[4]);
  mcp.endPacket();

  mcp.beginPacket(CANID_RC_56789);
  mcp.write(rcval[5]);
  mcp.write(rcval[6]);
  mcp.write(rcval[7]);
  mcp.write(rcval[8]);
  mcp.write(rcval[9]);
  mcp.endPacket();

  if (rcval[9] == 127) {
    int32_t f = (int8_t)rcval[1];
    int32_t r = (int8_t)rcval[0];
    int32_t t[4];
    int8_t m[4];
    mixer(f, r, 127, t, m);
    //Serial.printf("setWiper %03i %03i %03i %03i\n", t[MOTOR_L0], t[MOTOR_L1], t[MOTOR_R0], t[MOTOR_R1]);
    motor_driver(mcp, ds3502, m);
  } 


/*
  Serial.printf("%03i ", rcval[0]);
  Serial.printf("%03i ", rcval[1]);
  Serial.printf("%03i ", rcval[2]);
  Serial.printf("%03i ", rcval[3]);
  Serial.printf("%03i ", rcval[4]);
  Serial.printf("%03i ", rcval[5]);
  Serial.printf("%03i ", rcval[6]);
  Serial.printf("%03i ", rcval[7]);
  Serial.printf("%03i ", rcval[8]);
  Serial.printf("%03i ", rcval[9]);
  Serial.printf("\n");
  */

}