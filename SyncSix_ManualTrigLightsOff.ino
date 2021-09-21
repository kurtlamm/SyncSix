//SyncSix Manual Trig Light Off Sketch
//Channels 1-4 ('lights channels') are normally activated on
//Turns off 'lights channels' when trigger is activated. As long as trigger is activated controller will; 
//Wait "scareDelay" to activate channels 5-6 ('scare channels')
//Keep scare channels on for "scareTime" before turning off
//lights channels stay off untill tirgger is deactivated

const unsigned int scareDelay = 2000;    //define time (ms) controller waits to activate scare channels
const unsigned int scareTime = 3000;    //define time (ms) controller will activate scare channels

const bool invertTrig = LOW;    //Set to HIGH if using a normally closed trigger switch

#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <Wire.h>  

#define ambientbyte 0     //define arduino nano internal EEPROM settings bytes
#define volbyte 1


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
#define utrig A2                //unused in this particular sketch
#define uecho A3                //unused in this particular sketch
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



void setup() {
  pinMode(led, OUTPUT);  digitalWrite(led, LOW);      //setup LED
  pinMode(pot, INPUT);                                //setup potentiometer
  pinMode(busy, INPUT);                               //setup MP3 player feedback pin

  pinMode(ch1, OUTPUT); digitalWrite(ch1, HIGH);       //setup output channel pins and set all to LOW
  pinMode(ch2, OUTPUT); digitalWrite(ch2, HIGH);
  pinMode(ch3, OUTPUT); digitalWrite(ch3, HIGH);
  pinMode(ch4, OUTPUT); digitalWrite(ch4, HIGH);
  pinMode(ch5, OUTPUT); digitalWrite(ch5, LOW);
  pinMode(ch6, OUTPUT); digitalWrite(ch6, LOW);

  pinMode(SRload, OUTPUT);                            //setup input shift register pins
  pinMode(SRclocken, OUTPUT);
  pinMode(SRclock, OUTPUT);
  pinMode(SRdatain, INPUT);
  
  Wire.begin();                                       //begin I2C connection to external EEPROM

  DFSoftwareSerial.begin(9600);                       //begin serial connection to MP3 player
  byte MP3setvol[] = {0x7E, 0xFF, 0x06, 0x06, 0x00, 0x00, EEPROM.read(volbyte), 0xEF};     //read stored volume level
  DFSoftwareSerial.write(MP3setvol, 8);                //set volume of MP3 player
  
  digitalWrite(led, HIGH); delay(100); digitalWrite(led, LOW); delay(100); digitalWrite(led, HIGH); delay(100); digitalWrite(led, LOW); delay(100); digitalWrite(led, HIGH); delay(100); digitalWrite(led, LOW); delay(100); digitalWrite(led, HIGH); delay(100); digitalWrite(led, LOW); delay(100); digitalWrite(led, HIGH); delay(100); digitalWrite(led, LOW); delay(100); digitalWrite(led, HIGH); delay(100); digitalWrite(led, LOW); delay(100); //blink six times for startup confirmation
}



void loop() {
  inputCheck();   //refresh the inputs

  updateCh(0xF);    //turn on lights
  bool scareMP3 = HIGH;   //reset scareMP3 variable
  
  unsigned long startTime = millis();
  while (trigstate) {            
    unsigned long timePast = millis() - startTime;
    if (timePast > (scareDelay+scareTime)){
      updateCh(0x0);
    }
    else if (timePast > scareDelay){
      if (scareMP3){playMP3(1); scareMP3 = LOW;}      //play 1st mp3 file if hasnt been played this trigger cycle
      updateCh(0x30);
    }
    else {
      updateCh(0x0);
    }

    inputCheck();   //refresh the inputs
  }
  
  

  if (EEPROM.read(ambientbyte) && (analogRead(busy)>500)) {    //if play ambient track and ambient track has stoped
    playMP3(2);     //play 2nd MP3 (ambient)
    delay(25);
  }


  if (ch6state && shiftstate){     //toggle ambient enable eeprom bit if shift and ch6 are pressed
    if (EEPROM.read(ambientbyte) == 0){ EEPROM.write(ambientbyte, 1); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(1000); }
    else {EEPROM.write(ambientbyte, 0); digitalWrite(led, HIGH); delay(400); digitalWrite(led, LOW); delay(400); digitalWrite(led, HIGH); delay(400); digitalWrite(led, LOW); delay(400); digitalWrite(led, HIGH); delay(400); digitalWrite(led, LOW); delay(400); digitalWrite(led, HIGH); delay(400); digitalWrite(led, LOW); }
  }


  if (shiftstate  &&  ch5state) {    //if volume + is pressed
    byte newVol = constrain((EEPROM.read(volbyte) +1), 0, 30);      //read current volume from memory. Increment and constrain to volume range 0-30
    EEPROM.write(volbyte, newVol);                                  //write increased volume to arduino memory
    byte MP3setvol[] = {0x7E, 0xFF, 0x06, 0x06, 0x00, 0x00, newVol, 0xEF};     //create instruction to set new volume
    DFSoftwareSerial.write(MP3setvol, 8);                //set volume of MP3 player    
    digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW);
  }

  if (shiftstate  &&  ch4state) {    //if volume - is pressed
    byte newVol = constrain((EEPROM.read(volbyte) -1), 0, 30);      //read current volume from memory. Decrement and constrain to volume range 0-30
    EEPROM.write(volbyte, newVol);                                  //write decreased volume to arduino memory
    byte MP3setvol[] = {0x7E, 0xFF, 0x06, 0x06, 0x00, 0x00, newVol, 0xEF};     //create instruction to set new volume
    DFSoftwareSerial.write(MP3setvol, 8);                //set volume of MP3 player        
    digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW);
  }
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
    if (invertTrig){trigstate = LOW;}
  } else {trigstate = LOW; if (invertTrig){trigstate = HIGH;}}
}
