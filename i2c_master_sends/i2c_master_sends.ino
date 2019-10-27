#include <Wire.h> // <-- remove spaces

int slaveAddress = 9;

void setup()
{
  Wire.begin(); // join i2c bus (address optional for master)
}

void loop()
{
  Wire.beginTransmission(slaveAddress); // transmit to device #9
  Wire.write(1); // sends x
  Wire.endTransmission(); // stop transmitting
  delay(400);
  Wire.beginTransmission(slaveAddress); // transmit to device #9
  Wire.write(0); // sends x
  Wire.endTransmission(); // stop transmitting
  delay(400);
}
