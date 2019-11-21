/*
   IMPLEMENTATION OF SERIAL INTERACTION WITH OPENTHREAD CLI
   Ported code from: https://github.com/jaumecolomhernandez/autoconfig-openthread/blob/88c51c36a23c969f9de7fb898e71ded06d2342cc/device_classes.py#L37
*/

bool debug = false;
String endings[] = {"> ",
                    "> \n",
                    ">",
                    "Ú",
                   };
void setup() {
  Serial.begin(115200); // Serial setup
  Serial2.begin(115200); // Software serial
  Serial2.setRxBufferSize(2048);
  Serial2.println(".");  // This is needed to clean weird input symbols
  read_ans();
}
boolean isEnding(String string) {
  for (int i = 0; i < sizeof(endings); i++) {
    if (string.equals(endings[i])) {
      return true;
    }
    return false;
  }
}
String read_line() {
  /*
     Reads single line from Serial buffer
     Reads characters until \n and returns the string.
  */
  String data = "";
  int tries = 0;
  while (Serial2.available() == 0) {
    if (debug) Serial.println("[read_ans] The chosen UART Serial bus is not available"); // Not sure if needed
  }


  while (Serial2.available() > 0) {
    char character = Serial2.read(); // Receive a single character from the software serial port
    data.concat(character);           // Add the received character to the receive buffer
    if (character == '\n')
      return data;                    // Return the string
  }
}
void print_hex(String string) {
  /*
    Prints hexadecimal representation of string
  */
  String hex;
  char character = '-';

  for (int i = 0; i < string.length(); i++) {
    //Serial.print(string[i], HEX); // Iterates string and prints as HEX
    character = printf("%x", string[i] );
    hex.concat(character);
  }
  Serial.println(hex);
  Serial.println("");
}
void read_ans() {
  /*
     Reads full answer
     Reads lines until the value received matches with
     one of the defined endings.
  */

  int timeout_millis = 1000;
  int current_time = millis();

  bool wait = true;

  while (Serial2.available() == 0) {
    if (debug)
      Serial.println("[read_ans] The chosen UART Serial bus is not available"); // Not sure if needed
  }
  while (wait) {
    //we read each line received
    String recv_line = read_line();

    if (recv_line.length() != 0) {
      // *******************[DISTINGUISH STRING TYPES]**********************
      if (recv_line.endsWith("\n") && recv_line.length() == 2) {
        //string1 = recv_line
        //string2 = "\n"
        //However: ...
        //string 1 is not recognized as string2 with string1.equals(string2)
        //nothing to be 

        //not working either: string1.equals("\n\0")
      } else if (recv_line.endsWith("\n") ) {
        Serial.print("Received: ");
        Serial.print(recv_line);
        if (debug) {
          Serial.print("caseA");
          Serial.print(recv_line.length());
        }
      } else {
        Serial.print("Received: ");
        Serial.println(recv_line);
        if (debug) {
          Serial.print("caseB");
          Serial.print(recv_line.length());
        }
        // *******************************************************************
      }
      // if(debug) print_hex(recv_line);

      // STOP reading and displaying when reaching end of reception
      if (isEnding(recv_line)) {
        wait = false;
        if (debug) Serial.println("Now wait false");
      }
    }
    if (millis() > (current_time + timeout_millis)) wait = false;
  }
}
void send_command(String command) {
  Serial.println("\nSending: " + command);
  Serial2.println(command);
  read_ans();
}
void loop() {

  /*
    // *************************************************************
    send_command("help");
    delay(100);
    // *************************************************************
  */

  // **************[REPL] READ - EVAL - PRINT - LOOP**************
  String userCommand = "";
  if (Serial.available()) {
    userCommand = Serial.readStringUntil('\n');
    Serial.println(userCommand);
    send_command(userCommand);
  }
  delay(1000);
  // *************************************************************

}
