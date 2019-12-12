#include <Arduino.h>
#include "ot_cli_parser.h"

bool debug = false;
bool commissioner;

static Stream *zolertia; // Pointer to zolertia stream (serial)

 
void setSerial(Stream &serial){
  zolertia = &serial;
}

boolean isEnding(String string) {
  for (int i = 0; i < sizeof(endings); i++) {
    if (string.equals(endings[i])) {
      return true;
    }
    return false;
  }
}
String read_line(int t) {
  /*
     Reads single line from Serial buffer
     Reads characters until \n and returns the string.
  */
  String data = "";
  int tries = 0;
  if(t > 0){
    while (zolertia->available() == 0 && tries < t) {
      tries ++;
      if (debug) Serial.println("[read_ans] The chosen UART Serial bus is not available"); // Not sure if needed
    }
  }else{
    while (zolertia->available() == 0) {
      if (debug) Serial.println("[read_ans] The chosen UART Serial bus is not available"); // Not sure if needed
    }
  }
  

  while (zolertia->available() > 0) {
    char character = zolertia->read(); // Receive a single character from the software serial port
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
  bool done = false;
  bool wait = true;
  int finished = 0;

  while (zolertia->available() == 0) {
    if (debug)
      Serial.println("[read_ans] The chosen UART Serial bus is not available"); // Not sure if needed
  }
  while (wait) {
    //we read each line received
    String recv_line;
    if(done){
      finished++;
      recv_line = read_line(10);
    }else{
      recv_line = read_line();
    }
    if (recv_line.indexOf("Done") > -1){
      done = true;
    }
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
    if(finished > 15) wait = false;
  }
  Serial.println("Finished reading");
  return lines;
}

int send_command(String command, String answer[]) {
  Serial.println("\nSending: " + command);
  zolertia->println(command);
  int lines = read_ans(answer);
  return lines;
}


void start_commissioner() {
  String answer[MAX_LENGTH_ANSWER];
  for(int i = 0; i < length_init_commissioner_commands; i++){
  	send_command(init_commissioner_commands[i], answer);
    Serial.println(answer[2]);
      if(answer[2].indexOf("Invalid")>0){ //If the third line of the response has an error, restart configuration
        i=-1;
      }
      delay(7000);
  }
  commissioner = true;

}


void start_joiner() {
  String answer[MAX_LENGTH_ANSWER];
  for(int i = 0; i < length_init_joiner_commands; i++){
  	send_command(init_joiner_commands[i], answer);
    delay(7000);
    if( i == 0){ //TO DO: SCAN UNTIL NETWORK IS JOINABLE
      int length_answer;
      bool done = false;
      while(!done){
        Serial.println("Entering scanning"); 
        length_answer = read_ans(answer);
        Serial.println(length_answer);
        for(int j = 0; j < length_answer; j++){  
          if(answer[j].indexOf("Done")>-1){
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
   while(!joined && timeout < 10){
      length_answer = read_ans(answer);
      if(length_answer != 0){
        for(int i = 0; i < length_answer; i++){
          if(answer[i].indexOf("Join success")>0){
            joined = true;
          }else if(answer[i].indexOf("Join failed")>0){
            failed = true;
            break;
          }
        }
      }
      delay(10000);
      timeout++;
   }
   if(failed){
    send_command("joiner start AAAA", answer);
    delay(60000);
   }
}


void open_udp_communication(){
  String answer[MAX_LENGTH_ANSWER]={};
	send_command("udp open", answer);
	send_command("udp bind :: 1212", answer);
}

void udp_connect(String ip){
  String answer[MAX_LENGTH_ANSWER];
	send_command("udp open", answer);
	send_command("udp connect" + ip + "1212", answer);
}


void def_static_ip(int dev_id){
  String answer[MAX_LENGTH_ANSWER]={};
	send_command("ipaddr add dead:dead:cafe:cafe:dead:dead:cafe:000"+dev_id, answer);
}


void parse_neighbor_table(neighbor neighbors[]){
  String answer[MAX_LENGTH_ANSWER];
  int size = send_command("neighbors", answer);

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
