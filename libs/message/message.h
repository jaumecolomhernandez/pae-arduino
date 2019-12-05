#ifndef message_h
#define message_h

#include <message.h>
#include <Arduino.h>

struct message
{
  char header[10];
  char message[80];
};

enum State
{
  header,
  payload,
  none
};

void print_messages(struct message *msgs, int n_msgs);
void print_message(struct message mesg);
uint8_t parse_buffer(char *buffer, int nchars, struct message *msgs);
void buildMessage( char *buff, char message[], int mesg_len, char origin, char dest);

#endif 