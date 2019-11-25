#ifndef ot_cli_parser_h
#define ot_cli_parser_h

#include <Arduino.h>

const int MAX_NEIGHBORS = 15;
const int MAX_LENGTH_ANSWER = 25;

typedef struct{
  char role;
  String rloc;
  String mac;
}neighbor;

String answer[MAX_LENGTH_ANSWER] = {};
int read_ans(String command, String answer[] = answer);

String ip;
bool commissioner;
bool debug = false;
String endings[] = {"> ",
                    "> \n",
                    ">",
                    "Ú",
                   };

int length_init_commissioner_commands = 10;
String init_commissioner_commands[] = {"dataset init new",
									   "dataset meshlocalprefix dead:dead:cafe:cafe:dead:dead:cafe::",
									   "dataset",
									   "dataset commit active",
									   "panid 0xdead",
									   "ifconfig up",
									   "thread start",
									   "ipaddr",
									   "commissioner start",
									   "commissioner joiner add * AAAA",
									  };

int length_init_joiner_commands = 4;
String init_joiner_commands[] = {"ifconfig up",
								 "panid 0xdead",
								 "eui64",
								 "joiner start AAAA",						
								};

#endif 