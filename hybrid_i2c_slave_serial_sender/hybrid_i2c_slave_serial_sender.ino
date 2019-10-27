#include <Wire.h> // <-- remove spaces

int LED = 3;

int x = 0;

void setup()

{
  // Define the LED pin as Output
  pinMode (LED, OUTPUT);
  // Start the I2C Bus as Slave on address 9
  Wire.begin(9);
  // Attach a function to trigger when something is received.
  Wire.onReceive(receiveEvent);
  // Init serial
  Serial.begin(9600);
}

void receiveEvent( int bytes )
{
  // read one character from the I2C
  x = Wire.read(); 
  // And send via serial
  Serial.println(x);
}

void loop()
{
  
  if (x == 1) {
    digitalWrite(LED, HIGH);
    delay(100);
  }
  if (x == 0) {
    digitalWrite(LED, LOW);
    delay(100);
  }
  
}
