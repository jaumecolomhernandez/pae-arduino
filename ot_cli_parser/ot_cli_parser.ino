/*
   IMPLEMENTATION OF SERIAL INTERACTION WITH OPENTHREAD CLI
   Ported code from: https://github.com/jaumecolomhernandez/autoconfig-openthread/blob/88c51c36a23c969f9de7fb898e71ded06d2342cc/device_classes.py#L37
*/

#include "ot_cli_parser.h"


void setup() {
  String answer[MAX_LENGTH_ANSWER];
  Serial.begin(115200); // Serial setup
  Serial2.begin(115200); // Software serial
  Serial2.setRxBufferSize(2048);
  Serial2.println(".");  // This is needed to clean weird input symbols
  read_ans(answer);
  delay(5000);
  start_commissioner();
}

void loop() {

  /*
    // *************************************************************
    send_command("help");
    delay(100);
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

    
  // **************[REPL] READ - EVAL - PRINT - LOOP**************
  String userCommand = "";
  String answer[MAX_LENGTH_ANSWER]={};
  if (Serial.available()) {
    userCommand = Serial.readStringUntil('\n');
    Serial.println(userCommand);
    send_command(userCommand, answer);
  }
  delay(1000);
  // *************************************************************
  
  
}
