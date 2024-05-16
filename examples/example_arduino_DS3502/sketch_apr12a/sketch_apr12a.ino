#include <SPI.h>
#include <Wire.h>

#define I2C_SDA                          8      // I2C Data pin
#define I2C_SCL                          9      // I2C Clock pin

#define I2C_CLKRATE_400K            400000      // Speed of I2C bus 400KHz

#define DS3502_WIPER 0x00 ///< Wiper value register
#define DS3502_MODE 0x02  ///< Mode selection register
#define DS3502_I2CADDR_DEFAULT 0x28

void setup()
{
    Serial.begin(115200);
    while(!Serial);


    Wire.begin();                // Initialize I2C (Master Mode: address is optional)
    Wire.setClock(I2C_CLKRATE_400K);


  Serial.println("Mode set to 0x80: ");
  Wire.beginTransmission(DS3502_I2CADDR_DEFAULT);  // Transmit to device with address 44 (0x2C)
  Wire.write(DS3502_MODE);             // Sends value byte
  Wire.write(0x80);             // Sends value byte
  Wire.endTransmission();      // Stop transmitting
  delay(1000);
}


void read_mode()
{
  Wire.beginTransmission(DS3502_I2CADDR_DEFAULT);
  Wire.write(DS3502_WIPER);
  Wire.endTransmission(false);

  int n = Wire.requestFrom(DS3502_I2CADDR_DEFAULT, 1 , true );
  if(n != 0) {
    Serial.print("requestFrom Error!");
    Serial.print(n);
    Serial.println(".");
    return;
  }
  uint8_t b = Wire.read();
  Serial.print("Read register: ");
  Serial.print(b);
  Serial.println(".");
  delay(1000);
}


void loop()
{

  Serial.print("Wiper voltage with wiper set to 0: ");
  Wire.beginTransmission(DS3502_I2CADDR_DEFAULT);  // Transmit to device with address 44 (0x2C)
  Wire.write(DS3502_WIPER);             // Sends value byte
  Wire.write(0x00);             // Sends value byte
  Wire.endTransmission();      // Stop transmitting
  Serial.println("endTransmission.");
  delay(1000);

  Serial.print("Wiper voltage with wiper set to 50: ");
  Wire.beginTransmission(DS3502_I2CADDR_DEFAULT);  // Transmit to device with address 44 (0x2C)
  Wire.write(DS3502_WIPER);             // Sends value byte
  Wire.write(0x50);             // Sends value byte
  Wire.endTransmission();      // Stop transmitting
  Serial.println("endTransmission.");
  delay(1000);

  read_mode();

}

