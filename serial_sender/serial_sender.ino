/*
   IMPLEMENTATION OF SERIAL INTERACTION WITH OPENTHREAD CLI
   Ported code from: https://github.com/jaumecolomhernandez/autoconfig-openthread/blob/88c51c36a23c969f9de7fb898e71ded06d2342cc/device_classes.py#L37
*/
#include <SoftwareSerial.h>
SoftwareSerial mySerial(3, 4); // RX, TX
// TODO: Implement using real UARTS
// TODO: Think

#define LED 2


void setup() {
  pinMode(LED, OUTPUT); // Led setup
  Serial.begin(115200); // Serial setup
  while (!Serial)
  {}
  mySerial.begin(9600); // Software serial setup
  mySerial.println(".");  // This is needed to clean weird input symbols
}

String read_line() {
  /*
     Reads single line from Serial buffer
     Reads characters until \n and returns the string.
  */
  String Data = "";

  while (mySerial.available() == 0) {}// First waits until data in the buffer

  while (mySerial.available() > 0) {
    char character = mySerial.read(); // Receive a single character from the software serial port
    Data.concat(character);           // Add the received character to the receive buffer
    if (character == '\n')
      return Data;                    // Return the string
  }
}

void print_hex(String string) {
  /*
    Prints hexadecimal representation of string
  */
  for (int i = 0; i < string.length(); i++)
    Serial.print(string[i], HEX); // Iterates string and prints as HEX
  Serial.println("");
}

void read_ans() {
  /*
     Reads full answer
     Reads lines until the value received matches with
     one of the defined endings.
  */

  // TODO: Parse output
  // TODO: Add timeout
  // TODO: Add custom endings

  bool wait = true;

  while (mySerial.available() == 0) {}    // Not sure if needed
  while (wait) {
    String recv_data = read_line();

    Serial.print("Received: ");
    print_hex(recv_data);
    Serial.print(recv_data);

    if (recv_data == "> " || recv_data == "> \n") {
      wait = false;
      Serial.println("Now wait false");
    }
  }
}

void send_command(String command) {
    Serial.println("Sending: " + command);
    mySerial.println(command);
    read_ans();
}

void loop() {
  // TODO: Implement read–eval–print loop (REPL)
  send_command("help");
  delay(500);
  send_command("version");
  delay(500);
  send_command("wwwww");
  delay(10000);
}
