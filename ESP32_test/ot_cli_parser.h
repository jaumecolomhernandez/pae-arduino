#ifndef ot_cli_parser_h
#define ot_cli_parser_h

#include <Arduino.h>

/*************CONSTANTS DEFINITION*****************/
const int MAX_NEIGHBORS = 15;
const int MAX_LENGTH_ANSWER = 100;
const int length_init_commissioner_commands = 10;
const int length_init_joiner_commands = 6;

const String endings[] = {"> ",
                    "> \n",
                    ">",
                    "Ú",
					"failed",
                   };

const String init_commissioner_commands[] = {
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
					 //"ipaddr add dead:dead:cafe:cafe:dead:dead:cafe:0001",
                    };

const String init_joiner_commands[] = {
                 "scan",
                 "panid 0xdead",
                 "dataset meshlocalprefix dead:dead:cafe:cafe:dead:dead:cafe::",
                 "ifconfig up",
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
void setSerial(Stream &serial);
boolean isEnding(String string);
int read_ans(String answer[]);
String read_line(int t = 0);
void print_hex(String string);
int send_command(String command, String answer[]);
void start_commissioner();
void start_joiner();
void open_udp_communication();
void udp_connect(String ip);
void def_static_ip(int dev_id);
void parse_neighbor_table(neighbor neighbors[]);
#endif 
