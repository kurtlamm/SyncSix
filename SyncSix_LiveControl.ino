//SyncSix Live Controll Arduino Sketch. This sketch allows you to manualy manipulate the outputs of a SyncSix controller using the buttons.      
//The channel will be turned on while its coresponding button is pressed. You can invert the channel by holding SHIFT and pressing the desired channel button.
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

bool ch1inv = LOW;    //vars to store if channel has been iverted
bool ch2inv = LOW;
bool ch3inv = LOW;
bool ch4inv = LOW;
bool ch5inv = LOW;
bool ch6inv = LOW;


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
  
  if (ch1state && shiftstate) { //if channel invert button is pressed
    ch1inv = !ch1inv;         //set/reset invert flag
    delay(300);
  }
  if (ch2state && shiftstate) {
    ch2inv = !ch2inv;
    delay(300);
  }
  if (ch3state && shiftstate) {
    ch3inv = !ch3inv;
    delay(300);
  }
  if (ch4state && shiftstate) {
    ch4inv = !ch4inv;
    delay(300);
  }
  if (ch5state && shiftstate) {
    ch5inv = !ch5inv;
    delay(300);
  }
  if (ch6state && shiftstate) {
    ch6inv = !ch6inv;
    delay(300);
  }

    

  if (ch1inv) {           //if channel invert is set
    ch1state = !ch1state;   //invert the button state
  }
  if (ch2inv) {
    ch2state = !ch2state;
  }
  if (ch3inv) {
    ch3state = !ch3state;
  }
  if (ch4inv) {
    ch4state = !ch4state;
  }
  if (ch5inv) {
    ch5state = !ch5state;
  }
  if (ch6inv) {
    ch6state = !ch6state;
  }


   byte chanStates = 0; chanStates|=ch1state<<0; chanStates|=ch2state<<1; chanStates|=ch3state<<2; chanStates|=ch4state<<3; chanStates|=ch5state<<4; chanStates|=ch6state<<5;    //convert input states to byte representation
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
