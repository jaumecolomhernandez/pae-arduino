#include "Adafruit_FONA.h" // https://github.com/botletics/SIM7000-LTE-Shield/tree/master/Code
#include "ot_cli_parser.h"

// Define *one* of the following lines:
#define SIMCOM_7000E // SIM7000A/C/E/G

// For SIM7000 shield with ESP32
#define FONA_PWRKEY 18
#define FONA_RST 5
#define FONA_TX 16 // ESP32 hardware serial RX2 (GPIO16)
#define FONA_RX 17 // ESP32 hardware serial TX2 (GPIO17)

// Defines for zolertia
#define ZOL_TX 14 
#define ZOL_RX 27

// For the mesh network
#define MAX_NUM_NODES 10
#define ID	2
#define ID_STR '2'
#define PAYLOAD_SIZE 1024

// For ESP32 hardware serial
#include <HardwareSerial.h>
HardwareSerial fonaSS(1);
HardwareSerial zolertiaSS(2);

#include <message.h>
#define JOINER 0
#define COMMISSIONER 1
#define ROLE 1


Adafruit_FONA_LTE fona = Adafruit_FONA_LTE();

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);
uint8_t type;
char replybuffer[255]; // this is a large buffer for replies
char imei[16] = {0}; // MUST use a 16 character buffer for IMEI!
//unsigned long time;

struct node{
	char ipv6[45];
	char MAC[17];
};

struct node nodeList[MAX_NUM_NODES];

void addNode(int id, char ipv6[45], char MAC[17]){
	memcpy(nodeList[id].ipv6, ipv6, 45);
	memcpy(nodeList[id].MAC, MAC, 17);
}

uint8_t read_messages(struct message *msgs){
  /* Reads from receive buffer NON-BLOCKING */
  char replybuffer[255];
  uint16_t bytesRead = 0;
  uint16_t available = fona.UDPavailable();
  if (available > 0){
    printf("%d", available);
    bytesRead = fona.UDPread(replybuffer, 250);
  } else {
    return 0;
  }
  

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

  return n_msgs;
}

void setup() {
  //  while (!Serial);

  int role = ROLE;

  pinMode(FONA_RST, OUTPUT);
  digitalWrite(FONA_RST, HIGH); // Default state

  pinMode(FONA_PWRKEY, OUTPUT);

  // Turn on the module by pulsing PWRKEY low for a little bit
  // This amount of time depends on the specific module that's used
  powerOn(); // See function definition at the very end of the sketch

  Serial.begin(115200);
  Serial.println(F("ESP32 Basic Test"));
  Serial.println(F("Initializing....(May take several seconds)"));
  
  // Start at default SIM7000 shield baud rate
  fonaSS.begin(115200, SERIAL_8N1, FONA_TX, FONA_RX); // baud rate, protocol, ESP32 RX pin, ESP32 TX pin

  Serial.println(F("Configuring to 9600 baud"));
  fonaSS.println("AT+IPR=9600"); // Set baud rate
  delay(100); // Short pause to let the command run
  fonaSS.begin(9600, SERIAL_8N1, FONA_TX, FONA_RX); // Switch to 9600
  if (! fona.begin(fonaSS)) {
    Serial.println(F("Couldn't find FONA"));
    while (1); // Don't proceed if it couldn't find the device
  }

  type = fona.type();
  Serial.println(F("FONA is OK"));
  Serial.println(F("SIM7000E (European)")); 

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

  // Add all the nodes to the list
  addNode(1, "123123123", "213213123");

  Serial.println(nodeList[1].MAC);

  Serial.print(F("FONA> "));

  // NECESSARI AQUEST PARRAF?
  /*while (! Serial.available() ) { //Això fa que s'hagi de fer enter abans
    if (fona.available()) {
      Serial.write(fona.read());
    }
  }*/

    
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
  
  delay(200); // Needed delay

  char auth[] = "||AUTH 2|";
  if(! fona.UDPsend(auth, sizeof(auth)-1)) Serial.println(F("Failed to connect!"));
  /*char buffer[100];*/
  /*buildMessage(buffer, auth, sizeof(auth), '1', 'S');*/
  /*if(! fona.UDPsend(buffer, sizeof(buffer)-1)) Serial.println(F("Failed to connect!"));*/
  // Set Up the ZOlertia node
  String answer[MAX_LENGTH_ANSWER];
  zolertiaSS.begin(115200, SERIAL_8N1, ZOL_TX, ZOL_RX);
  zolertiaSS.setRxBufferSize(2048);
  setSerial(zolertiaSS);

  send_command(".", answer);  // This is needed to clean weird input symbols
  zolertiaSS.flush();

  if(role == COMMISSIONER){
	start_commissioner();
	//open_udp_communication();
  }else if(role == JOINER){
	start_joiner();
	send_command("ipaddr add dead:dead:cafe:cafe:dead:dead:cafe:0002", answer);
    udp_connect("dead:dead:cafe:cafe:dead:dead:cafe:0001");
	send_command("udp send dead:dead:cafe:cafe:dead:dead:cafe:0002", answer);
	open_udp_communication();
  }
}

long unsigned int time_ms = millis();
struct message msgs[10];

/*
*/
void send_t_time(int interval_ms){
  if ((millis() - time_ms) > interval_ms){
    time_ms = millis();
    if (! fona.UDPsend("ALIVE", sizeof("ALIVE")-1)) Serial.println(F("Failed to send!"));
    Serial.println(F("Sent 'ALIVE' "));
  }
}


int indexOf(char source[], char sub[], int len){
    int indexSub = 0;
    // printf("%d", len);
    
    // printf("%s\n",source);
    // printf("%s\n",sub);
    
    //printf("%d, %d\n", sizeof(sub),sizeof(source));
    
    for (int i=0; i<80; i++){
        if (source[i] == sub[indexSub]) {
            
            //printf("%d-%c\n",i, source[i]);
            indexSub = indexSub + 1;
            
            if (indexSub == (len-1)){
                return i-indexSub+1;    
            }
        } else {
            indexSub = 0;
        }
    }
    return -1;
}

void process_standard_message(char order[]){
  if (indexOf(order, "start_commissioner", sizeof("start_commissioner")) > -1){
    start_commissioner();
  } else if (indexOf(order,"start_joiner", sizeof("start_joiner")) > -1){
    start_joiner();
  } else if (indexOf(order,"open_udp_communication", sizeof("open_udp_communication")) > -1){
    open_udp_communication();
  } else if (indexOf(order,"udp_connect", sizeof("udp_connect")) > -1){
    char ip[39]; 
    memcpy(ip, order+11+1, 39); //to get IP
    udp_connect(ip);
  } else if (indexOf(order,"def_static_ip", sizeof("def_static_ip")) > -1){
    def_static_ip(int(order[14]));
  } else if (indexOf(order,"parse_neightbor_table", sizeof("parse_neightbor_table")) > -1){
    neighbor neighbors[10]; 
    parse_neighbor_table(neighbors);
  } else {
    printf("Order not implemented");
  }
}

void loop() {
  int role = ROLE;
  String answer[MAX_LENGTH_ANSWER];
  //send_command("help", answer);  // This is needed to clean weird input symbols*/
  /*zolertiaSS.flush();*/
  int lines;
  delay(1000);

  send_t_time(10000);
  
   
  // Read messages
  uint8_t n_msgs = read_messages(msgs); 
  printf("I have %i unhandled messages from the server\n", n_msgs);
  
  //print_messages(msgs, n_msgs);

  //Serial.println(F(""));

  // Take action
  for (int i = 0; i < n_msgs; i++){
    // Assumim que header sempre te llargada 0 o 1
    // if header len 0 is standard
    // if header Z is send to zolertia
    // if header A is ack 
    // if header S is standard
    // if (sizeof(msgs[i].header) > 1){
    //   printf("INVALID HEADER RECEIVED\n");
    //   printf("%d\n", sizeof(msgs[i].header));
    //   printf("%s\n", msgs[i].header);
    //   break;
    // }
    if (sizeof(msgs[i].header) == 0){
      printf("NO HEADER RECEIVED\n");
      process_standard_message(msgs[i].message);
    } else if (msgs[i].header[0] == 'Z'){
      printf("ZOLERTIA HEADER RECEIVED\n");
      send_command(msgs[i].message, answer);
    } else if (msgs[i].header[0] == 'C'){
      printf("ACK HEADER RECEIVED\n");
      fona.UDPsend("ACK", sizeof("ACK")-1); 
    } else if (msgs[i].header[0] == 'S'){
      printf("STANDARD HEADER RECEIVED\n");
      process_standard_message(msgs[i].message);
    } else {
      printf("NO KNOWN HEADER");
      process_standard_message(msgs[i].message);
    }
    // if (msgs[i].header[1] == 'A'){
    //   digestACK(msgs[i]);
    // } else if ( msgs[i].header[6] == '4' ){
    //   digestMessage(msgs[i]);
    // } else {
    //   fwdMessage(msgs[i]);
    // }
  }

  delay(100);
  if(role == COMMISSIONER){
	  // flush input
	  flushSerial();
	  while (fona.available()) {
		Serial.write(fona.read());
	  }

	  Serial.println("** Zolertia **");
	  lines = send_command(".", answer);
	  for(int i = 0; i<lines; i++){
		  Serial.println(answer[i]);
		  if(answer[i].indexOf("Joiner remove") > -1){
			  send_command("commissioner joiner add * AAAA", answer);
			  break;
		  }
	  }
	  
  }
}

void flushSerial() {
  while (Serial.available()) 
    Serial.read();
}

// Power on the module
void powerOn() {
  digitalWrite(FONA_PWRKEY, LOW);
  // See spec sheets for your particular module
  delay(100); // For SIM7000

  digitalWrite(FONA_PWRKEY, HIGH);
}

void digestMessage(struct message mesg){
	printf("***** DIGESTING MESSAGE YUMMY!! *****");
    String answer[MAX_LENGTH_ANSWER];
	printf(mesg.message);
  	send_command(mesg.message, answer);  // This is needed to clean weird input symbols 
	zolertiaSS.flush();
}

void digestACK(struct message mesg){

}

void fwdMessage(struct message mesg){

}
