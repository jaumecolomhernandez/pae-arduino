#include "Adafruit_FONA.h" // https://github.com/botletics/SIM7000-LTE-Shield/tree/master/Code

// Define *one* of the following lines:
#define SIMCOM_7000 // SIM7000A/C/E/G

// For SIM7000 shield with ESP32
#define FONA_PWRKEY 18
#define FONA_RST 5
#define FONA_TX 16 // ESP32 hardware serial RX2 (GPIO16)
#define FONA_RX 17 // ESP32 hardware serial TX2 (GPIO17)

// For the mesh network
#define MAX_NUM_NODES 10
#define ID	1
#define ID_STR '1'
#define PAYLOAD_SIZE 1024

// For ESP32 hardware serial
#include <HardwareSerial.h>
HardwareSerial fonaSS(1);

Adafruit_FONA_LTE fona = Adafruit_FONA_LTE();

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);
uint8_t type;
char replybuffer[255]; // this is a large buffer for replies
char imei[16] = {0}; // MUST use a 16 character buffer for IMEI!
//unsigned long time;

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

struct node{
	char ipv6[45];
	char MAC[17];
};

struct node nodeList[MAX_NUM_NODES];

void addNode(int id, char ipv6[45], char MAC[17]){
	memcpy(nodeList[id].ipv6, ipv6, 45);
	memcpy(nodeList[id].MAC, MAC, 17);
}

void print_messages(struct message *msgs, int n_msgs)
{
  // Prints the messages array
  for (int i = 0; i < n_msgs; i++)
  {
	printf("Message %i/%i - ", i, n_msgs);
	print_message(msgs[i]);
  }
}

void print_message(struct message mesg){
	printf("Header: %s - ", mesg.message);
	printf("Payload: %s\n", mesg.header);
}

uint8_t parse_buffer(char *buffer, int nchars, struct message *msgs)
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
  uint8_t n_msgs = 0;
  int begin = 0;
  int end = 0;

  s = none;

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

uint8_t read_messages(struct message *msgs){
  /* Reads from receive buffer NON-BLOCKING */
  char replybuffer[255];
  uint16_t bytesRead = fona.UDPread(replybuffer, 250);

  if (bytesRead == 0) {
    //Serial.println(F("Failed to read / nothing available!"));
  } else{
    Serial.println("Read content:");  
    for (int i=0; i<bytesRead; i++){
      Serial.print(replybuffer[i]);
    }
  }
  

  uint8_t n_msgs = parse_buffer(replybuffer, bytesRead, msgs);
  
  Serial.println(F(""));

  print_messages(msgs, n_msgs);

  Serial.println(F(""));

  // TODO: Implement protocol parser
  // Protocol is -> '||HEAD|CONTENT||'
  // TODO: Create functional script
  // DONE: Created testing script

  return n_msgs;
}

void setup() {
  //  while (!Serial);

  pinMode(FONA_RST, OUTPUT);
  digitalWrite(FONA_RST, HIGH); // Default state

  pinMode(FONA_PWRKEY, OUTPUT);

  // Turn on the module by pulsing PWRKEY low for a little bit
  // This amount of time depends on the specific module that's used
  powerOn(); // See function definition at the very end of the sketch

  Serial.begin(115200);
  Serial.println(F("ESP32 Basic Test"));
  Serial.println(F("Initializing....(May take several seconds)"));

  // Note: The SIM7000A baud rate seems to reset after being power cycled (SIMCom firmware thing)
  // SIM7000 takes about 3s to turn on but SIM7500 takes about 15s
  // Press reset button if the module is still turning on and the board doesn't find it.
  // When the module is on it should communicate right after pressing reset
  
  // Start at default SIM7000 shield baud rate
  fonaSS.begin(115200, SERIAL_8N1, FONA_TX, FONA_RX); // baud rate, protocol, ESP32 RX pin, ESP32 TX pin

  Serial.println(F("Configuring to 115200 baud"));
  fonaSS.println("AT+IPR=9600"); // Set baud rate
  delay(100); // Short pause to let the command run
  fonaSS.begin(9600, SERIAL_8N1, FONA_TX, FONA_RX); // Switch to 9600
  if (! fona.begin(fonaSS)) {
    Serial.println(F("Couldn't find FONA"));
    while (1); // Don't proceed if it couldn't find the device
  }

  type = fona.type();
  Serial.println(F("FONA is OK"));
  Serial.print(F("Found "));
  switch (type) {
    case SIM7000A:
      Serial.println(F("SIM7000A (American)")); break;
    case SIM7000C:
      Serial.println(F("SIM7000C (Chinese)")); break;
    case SIM7000E:
      Serial.println(F("SIM7000E (European)")); break;
    case SIM7000G:
      Serial.println(F("SIM7000G (Global)")); break;
    default:
      Serial.println(F("???")); break;
  }

  // Print module IMEI number.
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("Module IMEI: "); Serial.println(imei);
  }

  // Set modem to full functionality
  fona.setFunctionality(1); // AT+CFUN=1

  // Configure a GPRS APN, username, and password.
  fona.setNetworkSettings(F("orangeworld"), F("orange"), F("orange"));

  fona.setPreferredLTEMode(1); // Use LTE CAT-M only, not NB-IoT

}



void loop() {
  // Add all the nodes to the list
  addNode(1, "123123123", "213213123");

  Serial.println(nodeList[1].MAC);
  Serial.print(F("FONA> "));
  while (! Serial.available() ) {
    if (fona.available()) {
      Serial.write(fona.read());
    }
  }

    
  Serial.println("Starting autopilot!");

  // Wait until network status 2
  uint8_t n;
  do{
    n = fona.getNetworkStatus();
    Serial.print(F("The net status is"));
    Serial.println(n);
    delay(1000);
  }while(n != 1);

  // Activate GPRS
  if (!fona.enableGPRS(true)){
    Serial.println(F("Failed to turn on"));
  }

  // Connect to PAESERVER
  if (! fona.UDPconnect("147.83.39.50", 12342)) Serial.println(F("Failed to connect!")); 
  
  delay(200);

  char auth[7] = "AUTH  ";
  auth[5] = ID_STR;
  if(! fona.UDPsend(auth, sizeof(auth)-1)) Serial.println(F("Failed to connect!"));

  unsigned long time = millis();
  /*char hey[PAYLOAD_SIZE] = "HEY";*/

  while(true){

    /*// Send message every 4 seconds*/
    /*if ((millis() - time) > 4000){*/
      /*time = millis();*/
      /*if (! fona.UDPsend(hey, sizeof(hey)-1)) Serial.println(F("Failed to send!"));*/
      /*Serial.println(F("Sent message 'HEY' "));*/
    /*}*/

    struct message msgs[10];
    // Read messages
    uint8_t n_msgs = read_messages(msgs); 
	printf("I have %i unhandled messages", n_msgs);
    // Resonse callbacks
    /*for (int i=0; i<n_msgs; i++){*/
      /*if (msgs[i].message == "HELLO"){*/
        /*char res[] = "WORLD";*/
        /*if (! fona.UDPsend(res, sizeof(res))) Serial.println(F("Failed to send!"));*/
        /*Serial.println("Sent response");*/
      /*} else {*/
        /*Serial.println("Method not implemented");*/
      /*}*/
    /*}*/
	for (int i = 0; i < n_msgs; i++){
		if (msgs[i].header[1] == 'A'){
			digestMessage(msgs[i]);
		} else if ( msgs[i].header[6] == ID_STR ){
			digestACK(msgs[i]);
		} else {
			fwdMessage(msgs[i]);
		}
	}
    delay(1000);

    // flush input
    flushSerial();
    while (fona.available()) {
      Serial.write(fona.read());
    }
  }
}


void flushSerial() {
  while (Serial.available())
    Serial.read();
}

char readBlocking() {
  while (!Serial.available());
  return Serial.read();
}
uint16_t readnumber() {
  uint16_t x = 0;
  char c;
  while (! isdigit(c = readBlocking())) {
    //Serial.print(c);
  }sizeof
  Serial.print(c);
  x = c - '0';
  while (isdigit(c = readBlocking())) {
    Serial.print(c);
    x *= 10;
    x += c - '0';
  }
  return x;
}

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout) {
  uint16_t buffidx = 0;
  boolean timeoutvalid = true;
  if (timeout == 0) timeoutvalid = false;

  while (true) {
    if (buffidx > maxbuff) {
      //Serial.println(F("SPACE"));
      break;
    }

    while (Serial.available()) {
      char c =  Serial.read();

      //Serial.print(c, HEX); Serial.print("#"); Serial.println(c);

      if (c == '\r') continue;
      if (c == 0xA) {
        if (buffidx == 0)   // the first 0x0A is ignored
          continue;

        timeout = 0;         // the second 0x0A is the end of the line
        timeoutvalid = true;
        break;
      }
      buff[buffidx] = c;
      buffidx++;
    }

    if (timeoutvalid && timeout == 0) {
      //Serial.println(F("TIMEOUT"));
      break;
    }
    delay(1);
  }
  buff[buffidx] = 0;  // null term
  return buffidx;
}

// Power on the module
void powerOn() {
  digitalWrite(FONA_PWRKEY, LOW);
  // See spec sheets for your particular module
#if defined(SIMCOM_2G)
  delay(1050);
#elif defined(SIMCOM_3G)
  delay(180); // For SIM5320
#elif defined(SIMCOM_7000)
  delay(100); // For SIM7000
#elif defined(SIMCOM_7500)
  delay(500); // For SIM7500
#endif

  digitalWrite(FONA_PWRKEY, HIGH);
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


void digestMessage(struct message mesg){
	print_message(mesg);
}

void digestACK(struct message mesg){

}

void fwdMessage(struct message mesg){

}
