#include <stdio.h>
#include <string.h>

enum State
{
    header,
    payload,
    none
};

struct message
{
    char header[10];
    char message[80];
} new_msg = {{0}, {0}};

enum State s = none;

void print_messages(struct message *msgs, int n_msgs)
{
    // Prints the messages array
    for (int i = 0; i < n_msgs; i++)
    {
        printf("Message %i/%i - ", i, n_msgs);
        printf("Header: %s - ", msgs[i].header);
        printf("Payload: %s\n", msgs[i].message);
    }
}

int parse_buffer(char *buffer, int nchars, struct message *msgs)
{
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
    int n_msgs = 0;
    int begin = 0;
    int end = 0;

    for (int i = 0; i < nchars; i++)
    {
        if ((s == none) && (buffer[i] == '|'))
        {
            // Change state to header (none->header)
            s = header;             // Set state
            begin = i + 1;          // Set begin index of message
            msgs[n_msgs] = new_msg; // Initialize with null arrays
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

int main()
{
    char buffer[] = "|HEDF|MESSAGE||DE|CLAR|";
    struct message msgs[10];

    int n_msgs = parse_buffer(buffer, sizeof(buffer), msgs);

    print_messages(msgs, n_msgs);

    return 0;
}
