/*
   IMPLEMENTATION OF SERIAL INTERACTION WITH OPENTHREAD CLI
   Ported code from: https://github.com/jaumecolomhernandez/autoconfig-openthread/blob/88c51c36a23c969f9de7fb898e71ded06d2342cc/device_classes.py#L37
*/
typedef struct{
  char role;
  String rloc;
  String mac;
}neighbor;

const int MAX_NEIGHBORS = 15;
const int MAX_LENGTH_ANSWER = 25;
bool debug = false;
String endings[] = {"> ",
                    "> \n",
                    ">",
                    "Ú",
                   };
void setup() {
  //String answer[50];
  Serial.begin(115200); // Serial setup
  Serial2.begin(115200); // Software serial
  Serial2.setRxBufferSize(2048);
  Serial2.println(".");  // This is needed to clean weird input symbols
  //read_ans(answer);
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
int read_ans(String answer[]) {
  /*
     Reads full answer
     Reads lines until the value received matches with
     one of the defined endings.
  */
  int lines = 0;
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
      answer[lines] = recv_line;
      
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
      lines++;
      // STOP reading and displaying when reaching end of reception
      if (isEnding(recv_line)) {
        wait = false;
        if (debug) Serial.println("Now wait false");
      }
     
    }
    if (millis() > (current_time + timeout_millis)) wait = false;
  }
  return lines;
}
int send_command(String command, String answer[]) {
  Serial.println("\nSending: " + command);
  Serial2.println(command);
  return lines = read_ans(answer);
}
void parse_neighbor_table(String answer[], int size, neighbor neighbors[]){
  int count = 0;
  for(int i = 3; i < size-2; i++){
    neighbor n;
    Serial.println(answer[i].charAt(4));
    n.role = answer[i].charAt(4);
    n.rloc = answer[i].substring(9,15);
    n.mac = answer[i].substring(55, 71);
    neighbors[count] = n;
    count++;
  }
   
}

void loop() {

  /*
    // *************************************************************
    send_command("help");
    delay(100);
    // *************************************************************
  */
/*
  // **************[REPL] READ - EVAL - PRINT - LOOP**************
  String userCommand = "";
  String answer[50];
  if (Serial.available()) {
    userCommand = Serial.readStringUntil('\n');
    Serial.println(userCommand);
    send_command(userCommand, answer);
  }
  delay(1000);
  // *************************************************************
*/
  /*TEST
  Serial.println("TESTING");
  neighbor neighbors[3];
  String ans[8] = {"neighbor table", "| Role | RLOC16 | Age | Avg RSSI | Last RSSI |R|S|D|N| Extended MAC     |",
                      "+--------+-----+----------+-----------+-+-+-+-+------------------+", 
                      "|   C  | 0xcc01 |  96 |      -46 |       -46 |1|1|1|1| 1eb9ba8a6522636b |",
                      "|   R  | 0xc800 |   2 |      -29 |       -29 |1|0|1|1| 9a91556102c39ddb |",
                      "|   R  | 0xf000 |   3 |      -28 |       -28 |1|0|1|1| 0ad7ed6beaa6016d |",
                      "Done", "\n"};
  Serial.println(ans[3].charAt(4));
  
  parse_neighbor_table(ans, 8, neighbors);
  delay(1000);
  for(int i = 0; i < 3; i++){
    Serial.println("Neighbor");
    Serial.println(neighbors[i].role);
    Serial.println(neighbors[i].rloc);
    Serial.println(neighbors[i].mac);
  }
  */
  neighbor neigbhbors[MAX_NEIGHBORS];
  String answer[MAX_LENGTH_ANSWER];
  int length_answer;
  int length_answer = send_command("neighbor table", answer);
  parse_neighbor_table(answer, length_answer, neighbors);
}
