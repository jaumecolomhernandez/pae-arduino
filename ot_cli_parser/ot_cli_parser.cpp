#include <Arduino.h>
#include "ot_cli_parser.h"

bool debug = false;
bool commissioner;
String test[MAX_LENGTH_ANSWER];
neighbor neighbors[MAX_NEIGHBORS];
String answer[MAX_LENGTH_ANSWER];

// Reads single line from Serial buffer
// Returns a String that contains all the characters in the buffer, until there is a '\n'
String read_line(int t) {

  String data = "";

  while (Serial2.available() > 0) {
    // Receive a single character from the software serial port
    char character = Serial2.read();
    // Add the received character to the receive buffer
    data.concat(character);
    // If there is a '\n' return all the collected data
    if (character == '\n'){
      //print_hex(data);
      return data;
    }
    // Waits until there is data in buffer
    while (Serial2.available() == 0){}
  }
}

// Reads a full answer. Reads lines until the value received matches with one of the defined endings
int read_ans() {
  
  int lines = 0;
  bool wait = true;

  while (wait) {
    // Waits for data in buffer
    while (Serial2.available() == 0) {}
    // We read each line received
    answer[lines]= read_line();
    // If there is a 'Done' or an error, stops reading the answer
    if (answer[lines].indexOf("Done") > -1 || answer[lines].indexOf("Error") > -1) wait = false;
    // Prints the line on screen
    Serial.print("Received: ");
    Serial.print(answer[lines]);
    lines++;
  }
  // Returns the number of lines that had been read
  return lines;
}

// Prints hexadecimal representation of a string
void print_hex(String string) {

  String hex;
  char character = '-';

  // For each character in the String, transforms the character to HEX and add it to 'hex'
  for (int i = 0; i < string.length(); i++) {
    Serial.print(string[i], HEX);
    Serial.print(character);
  }
  // Prints the string in HEX format
  Serial.println();
}

void print_line() {
  String line = read_line().substring(2, 24);
  Serial.print(line);
  print_hex(line);
}

// Reads the aswer if a 'help' or '\n' had been sended
void read_help() {
  bool hasEnded = false;
  String line;

  // Waits for data in buffer
  while (Serial2.available() == 0) {}
  while (!hasEnded) {
    line = read_line();
    // Note that the help command no returns 'Done', returns '>'
    if (line.indexOf(">") > -1 || line.indexOf("Error") > -1){
      hasEnded = true;
      break;
    }
    Serial.print("Received: ");
    Serial.print(line);
  }

}

int send_command(String command) {
  // Flush the buffer
  while (Serial2.available()) {
    Serial2.read();
  }
  // Prints the command on screen
  Serial.println("\nSending: " + command);
  // Sends the command
  Serial2.println(command);
  // Reads the aswer if a 'help' or '\n' had been sended
  if (command == "help" || command == "") read_help();
  else {
    int lines = read_ans();
    return lines;
  }
}

// Initializes the Zolertia as commissioner, and commissions the network
void start_commissioner() {
  // Sends each of init_commissioner_commands commands. See 'init_joiner_commands[]' in ot_cli_parser.h
  for (int i = 0; i < length_init_commissioner_commands; i++) {
    send_command(init_commissioner_commands[i]);
    // If had sent the 'commissioner start' command
    if (init_commissioner_commands[i] == "commissioner start") {
      // Wait for the second part of the answer
      while (!Serial2.available()) {}
      // Prints the second part of the answer
      print_line();
    }
    // If the third line of the response has an error, restart configuration
    if (answer[2].indexOf("Invalid") > 0)  i = -1;
    delay(3000);
  }
  commissioner = true;
}


void start_joiner() {
  String answer[MAX_LENGTH_ANSWER];
  for (int i = 0; i < length_init_joiner_commands; i++) {
    send_command(init_joiner_commands[i]);
    delay(7000);
    if ( i == 0) { //TO DO: SCAN UNTIL NETWORK IS JOINABLE
      int length_answer;
      bool done = false;
      while (!done) {
        Serial.println("Entering scanning");
        length_answer = read_ans();
        Serial.println(length_answer);
        for (int j = 0; j < length_answer; j++) {
          if (answer[j].indexOf("Done") > -1) {
            done = true;
            Serial.println("Finished scanning");
          }
        }
      }
    }

  }
  /*
    bool join_answer = false;
    //Wait for an answer
    //Check if is a Join success answer
    length_answer = read_ans(answer);
    Serial.print("Answer to the joining: ");
    for(int i = 0; i<length_answer; i++){
      Serial.println(answer[i]);
      if(answer[i].equals("Join success")){
        join_answer=true;
        send_command("thread start", answer);
        delay(5000);
      }
    }
    if(!join_answer){
       Serial.println("Join failure");
    }
    commissioner = false;
  */
  bool joined = false;
  bool failed = false;
  int length_answer = 0;
  int timeout = 0;
  while (!joined && timeout < 10) {
    length_answer = read_ans();
    if (length_answer != 0) {
      for (int i = 0; i < length_answer; i++) {
        if (answer[i].indexOf("Join success") > 0) {
          joined = true;
        } else if (answer[i].indexOf("Join failed") > 0) {
          failed = true;
          break;
        }
      }
    }
    delay(10000);
    timeout++;
  }
  if (failed) {
    send_command("joiner start AAAA");
    delay(60000);
  }
}


void open_udp_communication() {
  String answer[MAX_LENGTH_ANSWER] = {};
  send_command("udp open");
  send_command("udp bind :: 1212");
}

void udp_connect(String ip) {
  String answer[MAX_LENGTH_ANSWER];
  send_command("udp open");
  send_command("udp connect" + ip + "1212");
}


void def_static_ip(int dev_id) {
  String answer[MAX_LENGTH_ANSWER] = {};
  send_command("ipaddr add dead:dead:cafe:cafe:dead:dead:cafe:000" + dev_id);
}


void parse_neighbor_table(String answer[], int size, neighbor neighbors[]) {
  int count = 0;
  for (int i = 3; i < size - 2; i++) {
    neighbor n;
    Serial.println(answer[i].charAt(4));
    n.role = answer[i].charAt(4);
    n.rloc = answer[i].substring(9, 15);
    n.mac = answer[i].substring(55, 71);
    neighbors[count] = n;
    count++;
  }

}

void fake_answer() {
  test[0] = String("> neighbor table") + String("\n") + String("\r");
  test[1] = String("| Role | RLOC16 | Age | Avg RSSI | Last RSSI |R|S|D|N| Extended MAC     |") + String("\n") + String("\r");
  test[2] = String("+------+--------+-----+----------+-----------+-+-+-+-+------------------+") + String("\n") + String("\r");
  test[3] = String("|   C  | 0xcc01 |  96 |      -46 |       -46 |1|1|1|1| 1eb9ba8a6522636b |") + String("\n") + String("\r");
  test[4] = String("|   R  | 0xc800 |   2 |      -29 |       -29 |1|0|1|1| 9a91556102c39ddb |") + String("\n") + String("\r");
  test[5] = String("|   R  | 0xf000 |   3 |      -28 |       -28 |1|0|1|1| 0ad7ed6beaa6016d |") + String("\n") + String("\r");
  test[6] = String("Done") + String("\n") + String("\r");
  for (int i = 0; i < 7; i++) {
    Serial.println(test[i]);
    print_hex(test[i]);
  }
}

// Reads the neighbor table response and writes neighbors[] properly
void read_neighbor_table() {
  send_command("neighbor table");
  // Dissmis the three first lines
  int i = 2;
  // Finds the information until there is a 'Done'
  while (answer[i] != "Done" + String("\n") + String("\r")) {
    // Defines a neighbor struct variable
    neighbor n;
    Serial.println(i);
    Serial.println(answer[i].charAt(4));
    Serial.println(answer[i].substring(9, 15));
    Serial.println(answer[i].substring(55, 71));
    // Fills the nieghbor fields
    n.role = test[i].charAt(4);
    n.rloc = test[i].substring(9, 15);
    n.mac = test[i].substring(55, 71);
    // Adds the neighbor to the array of all the neighbors of the node
    neighbors[i] = n;
    i++;
  }
}

int read_channel_rssi(){
  Serial2.println("scan energy 100");
  bool wait = true;
  String recv_line;
  int lines = 0;
  String channels_rssi[16][2];

  while (wait) {
    // Waits for data in buffer
    while (Serial2.available() == 0) {}
    // We read each line received
    recv_line = read_line();
    // If there is a 'Done' or an error, stops reading the answer
    if (recv_line.indexOf("Done") > -1 || recv_line.indexOf("Error") > -1) wait = false;
    // Stores the line in answer
    answer[lines] = recv_line;
    // Prints the line on screen
    Serial.print("Received: ");
    Serial.print(recv_line);
    // Stores a vector in 'channels_rssi' with the channel number and the rssi value
    if(lines > 4) channels_rssi[lines-5][0] = lines+6;
    // At the fourth line there is a '>' simbol, which means 'substring()' must have diferent parameters
    if(lines == 4)  channels_rssi[lines-4][1] = recv_line.substring(12, 13);
    if(lines > 4 && lines != 20)  channels_rssi[lines-4][1] = recv_line.substring(10, 11);
    lines++;
  }
  int i = 0;
  while(i<16){
    Serial.println(i);
    Serial.println(channels_rssi[i][0]);
    Serial.println(channels_rssi[i][1]);
    i++;
  }
  // Returns the number of lines that had been read
  return lines;
}
