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
int read_ans(String answer[] = answer);

bool commissioner;
bool debug = false;
String endings[] = {"> ",
                    "> \n",
                    ">",
                    "Ú",
                   };


#endif 