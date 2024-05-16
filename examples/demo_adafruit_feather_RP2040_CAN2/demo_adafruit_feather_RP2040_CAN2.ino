/*
 * Adafruit MCP2515 FeatherWing CAN Receiver Example
 */

#include <Adafruit_MCP2515.h>
#include <Adafruit_DS3502.h>
#include "config.h"
#include "progress_can.hpp"
#include "progress_rc.hpp"
#include "progress_motor.hpp"
#include "progress_mixer.hpp"
#include "ibus.hpp"
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

  // Wait for serial connection:
  Serial.begin(115200);
  for(int i = 0; i < 100; ++i){
    if(Serial) {
      break;
    }
    delay(10);
  }

  Serial.printf("[watchrebooted] %i\n", watchrebooted);


  // Restart MCU if watchdog is not updated
  watchdog_enable((0x7fffff / 8) / 10, false);
  watchdog_start_tick(12);


  add_repeating_timer_ms(1000, repeating_timer_callback, NULL, &timer);



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


app_t app = {0};


void loop() {
  IBus.loop();
  progress_can(&app, mcp);
  progress_rc(&app);
  progress_mixer(&app);
  progress_motor(&app, ds3502);
  // Test if watchdog by triggering it from RC handcontroller:
  if (app.rcvals[8] < 0) {
    watchdog_update();
  }
}