#ifndef ot_cli_parser_h
#define ot_cli_parser_h

#include <Arduino.h>

/*************CONSTANTS DEFINITION*****************/
const int MAX_NEIGHBORS = 15;
const int MAX_LENGTH_ANSWER = 100;
const int length_init_commissioner_commands = 11;
const int length_init_joiner_commands = 4;

const String endings[] = {"> ",
                    "> \n",
                    ">",
                    "Ú",
                   };

const String init_commissioner_commands[] = {"help",
                      "dataset init new",
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

const String init_joiner_commands[] = {"ifconfig up",
                 "panid 0xdead",
                 "eui64",
                 "joiner start AAAA",           
                };


/*************VARIABLES DECLARATION*****************/

typedef struct{
  char role;
  String rloc;
  String mac;
}neighbor;


/**************FUNCTIONS DECLARATION***************/
boolean isEnding(String string);
int read_ans(String answer[]);
String read_line();
void print_hex(String string);
int send_command(String command, String answer[]);
void start_commissioner();
void start_joiner();
void open_udp_communication();
void udp_connect(String ip);
void def_static_ip(int dev_id);
void parse_neighbor_table(String answer[], int size, neighbor neighbors[]);
#endif 
