#include <message.h>
#include <Arduino.h>

/*    
*/
void print_messages(struct message *msgs, int n_msgs){
  for (int i = 0; i < n_msgs; i++){
    printf("Message %i/%i - ", i, n_msgs);
    print_message(msgs[i]);
  }
}

/*
*/
void print_message(struct message mesg){
	printf("Header: %s - ", mesg.message);
	printf("Payload: %s\n", mesg.header);
}


/* Parses the receiver buffer 
    The protocol is the following |HEADER|PAYLOAD| the messages 
    are separated using vertical bars (|)
    Params:
    - buffer: (*char) pointer to char array containing the received content
    - nchars: (int) lenght of the buffer
    - msgs: (struct *message) pointer to array of messages(struct)
    Returns:
    - n_msgs: (int) number of messages received
*/
uint8_t parse_buffer(char *buffer, int nchars, struct message *msgs)
{
  uint8_t n_msgs = 0;
  int begin = 0;
  int end = 0;
  enum State s = none;

  for (int i = 0; i < nchars; i++)
  {
      if ((s == none) && (buffer[i] == '|'))
      {
          // Change state to header (none->header)
          s = header;             // Set state
          begin = i + 1;          // Set begin index of message
          msgs[n_msgs] = {{0}, {0}}; // Initialize with null arrays
      }
      else if ((s == header) && (buffer[i] == '|'))
      {
          // Change state to payload (header->payload)
          s = payload; // Set state
          end = i;     // Set end index of header

          // Copy from index begin to index end (both inclusive)
          memcpy(msgs[n_msgs].header, buffer + begin, end - begin);

          begin = i + 1; // Set begin index of payload
      }
      else if ((s == payload) && (buffer[i] == '|'))
      {
          // Change state back to none (payload -> none)
          end = i;  // Set payload end index
          s = none; // Set state

          // Copy from index begin to index end (both inclusive)
          memcpy(msgs[n_msgs].message, buffer + begin, end - begin);

          // Add one to number of messages
          n_msgs = n_msgs + 1;
      }
  }
  return n_msgs;
}

void buildMessage( char *buff, char message[], int mesg_len, char origin, char dest ){
// USAR SPRINTF
  char header[] = "|O :D: |";
  header[2] = origin;
  header[6] = dest;

  memcpy(buff, header, sizeof(header));
  strcat(buff, message);
  strcat(buff, "|");
}