//SyncSix Toggle Controll Arduino Sketch. This sketch allows you to manualy toggle the outputs of a SyncSix controller using the buttons.      
//The channel will be turned on/off if its coresponding button is pressed. 
//The playback audio track (first track) can be triggered by pressing the TRIG button. 

#include <SoftwareSerial.h>

#define SRload 5          //define pin connections
#define SRdatain 2
#define SRclock 4
#define SRclocken A1
#define ch1 A0
#define ch2 10
#define ch3 9
#define ch4 8
#define ch5 7
#define ch6 6
#define led 13
#define utrig A2
#define uecho A3
#define pot A7
#define busy A6
#define tx 11
#define rx 12

SoftwareSerial DFSoftwareSerial(rx, tx);                                            // RX, TX of MP3 player
const byte playMP31[] = {0x7E, 0xFF, 0x06, 0x03, 0x00, 0x00, 0x01, 0xEF};           //message to send to MP3 player to play track 1
const byte playMP32[] = {0x7E, 0xFF, 0x06, 0x03, 0x00, 0x00, 0x02, 0xEF};           //message to send to MP3 player to play track 2

bool ch1state;    //global vars to store input states
bool ch2state;
bool ch3state;
bool ch4state;
bool ch5state;
bool ch6state;
bool shiftstate;
bool trigstate;
bool sonicstate;

bool ch1tog = LOW;    //vars to store if channel has been toggeled on
bool ch2tog = LOW;
bool ch3tog = LOW;
bool ch4tog = LOW;
bool ch5tog = LOW;
bool ch6tog = LOW;


void setup() {
  pinMode(led, OUTPUT);  digitalWrite(led, LOW);      //setup LED
  pinMode(pot, INPUT);                                //setup potentiometer
  pinMode(busy, INPUT);                               //setup MP3 player feedback pin


  pinMode(ch1, OUTPUT); digitalWrite(ch1, LOW);       //setup output channel pins and set all to LOW
  pinMode(ch2, OUTPUT); digitalWrite(ch2, LOW);
  pinMode(ch3, OUTPUT); digitalWrite(ch3, LOW);
  pinMode(ch4, OUTPUT); digitalWrite(ch4, LOW);
  pinMode(ch5, OUTPUT); digitalWrite(ch5, LOW);
  pinMode(ch6, OUTPUT); digitalWrite(ch6, LOW);

  pinMode(SRload, OUTPUT);                            //setup input shift register pins
  pinMode(SRclocken, OUTPUT);
  pinMode(SRclock, OUTPUT);
  pinMode(SRdatain, INPUT);
  

  DFSoftwareSerial.begin(9600);                       //begin serial connection to MP3 player
  byte MP3setvol[] = {0x7E, 0xFF, 0x06, 0x06, 0x00, 0x00, 30, 0xEF};     //read stored volume level
  DFSoftwareSerial.write(MP3setvol, 8);                //set volume of MP3 player

  digitalWrite(led, HIGH); delay(100); digitalWrite(led, LOW); delay(100); digitalWrite(led, HIGH); delay(100); digitalWrite(led, LOW); delay(100); digitalWrite(led, HIGH); delay(100); digitalWrite(led, LOW); delay(100); digitalWrite(led, HIGH); delay(100); digitalWrite(led, LOW); delay(100); digitalWrite(led, HIGH); delay(100); digitalWrite(led, LOW); delay(100); digitalWrite(led, HIGH); delay(100); digitalWrite(led, LOW); delay(100); //blink six times for startup confirmation
}



void loop() {
  inputCheck();   //refresh the inputs

  if (trigstate) {            //if triggered, play audio file
    playMP3(1);  //play first audio file
    delay(300);
  }
  
  if (ch1state) { //if channel button is pressed
    ch1tog = !ch1tog;         //toggle channel
    delay(200);
  }
  if (ch2state) { 
    ch2tog = !ch2tog;        
    delay(200);
  }
  if (ch3state) {
    ch3tog = !ch3tog;         
    delay(200);
  }  
  if (ch4state) { 
    ch4tog = !ch4tog;        
    delay(200);
  }
  if (ch5state) { 
    ch5tog = !ch5tog;         
    delay(200);
  }
  if (ch6state) { 
    ch6tog = !ch6tog;       
    delay(200);
  }  
  

   byte chanStates = 0; chanStates|=ch1tog<<0; chanStates|=ch2tog<<1; chanStates|=ch3tog<<2; chanStates|=ch4tog<<3; chanStates|=ch5tog<<4; chanStates|=ch6tog<<5;    //convert input states to byte representation
   updateCh(chanStates);    //update channel outputs
}




void updateCh(byte states) {            //updates output channels
  if (bitRead(states, 5) == 1) {digitalWrite(ch6, HIGH);}
  else {digitalWrite(ch6, LOW);}

  if (bitRead(states, 4) == 1) {digitalWrite(ch5, HIGH);}
  else {digitalWrite(ch5, LOW);}
  
  if (bitRead(states, 3) == 1) {digitalWrite(ch4, HIGH);}
  else {digitalWrite(ch4, LOW);}
  
  if (bitRead(states, 2) == 1) {digitalWrite(ch3, HIGH);}
  else {digitalWrite(ch3, LOW);}
  
  if (bitRead(states, 1) == 1) {digitalWrite(ch2, HIGH);}
  else {digitalWrite(ch2, LOW);}

  if (bitRead(states, 0) == 1) {digitalWrite(ch1, HIGH);}
  else {digitalWrite(ch1, LOW);}
}



void playMP3(unsigned int track) {        //plays MP3 tracks
  switch (track) {
    case 1:
          DFSoftwareSerial.write(playMP31, 8);      //send code to play 1st MP3
          break;
    case 2:
          DFSoftwareSerial.write(playMP32, 8);      //send code to play 2nd MP3
          break;
  }
}



void inputCheck() {         //refreshes input button states and trig
  digitalWrite(SRload, LOW);  // Write pulse to load pin of 74HC165 shift register
  delayMicroseconds(5);
  digitalWrite(SRload, HIGH);
  delayMicroseconds(5);
 
  digitalWrite(SRclock, HIGH);  // Get data from 74HC165 shift register
  digitalWrite(SRclocken, LOW);
  byte incoming = shiftIn(SRdatain, SRclock, LSBFIRST);     //recieve incoming byte from shift register
  digitalWrite(SRclocken, HIGH);

  if (!(incoming & 0x80 )) {            //set channel states based on input buttons
    ch1state = HIGH;
  } else ch1state = LOW;
  
  if (!(incoming &  0x40 )) {
    ch2state = HIGH;
  } else ch2state = LOW;

  if (!(incoming &  0x20 )) {
    ch3state = HIGH;
  } else ch3state = LOW;

  if (!(incoming &  0x10 )) {
    ch4state = HIGH;
  } else ch4state = LOW;

  if (!(incoming &  0x8 )) {
    ch5state = HIGH;
  } else ch5state = LOW;

  if (!(incoming &  0x4 )) {
    ch6state = HIGH;
  } else ch6state = LOW;

  if (!(incoming &  0x2 )) {
    shiftstate = HIGH;
  } else shiftstate = LOW;

  if (!(incoming &  0x1 )) {
    trigstate = HIGH;
  } else trigstate = LOW;

}
