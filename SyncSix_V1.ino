//SyncSix V1 Arduino Sketch      
const unsigned long quant = 75;      //set quantization time constant (how often outputs are refreshed in ms). Default 75ms.  Min 25ms.


#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <Wire.h>  

#define ambientbyte 0     //define arduino nano internal EEPROM settings bytes
#define volbyte 1
#define sonicbyte 2

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




void setup() {
  pinMode(led, OUTPUT);  digitalWrite(led, LOW);      //setup LED
  //pinMode(pot, INPUT);                                //setup potentiometer
  //pinMode(busy, INPUT);                               //setup MP3 player feedback pin

  pinMode(utrig, OUTPUT); digitalWrite(utrig, LOW);   // Set the ultrasonic trigPin as an OUTPUT and set LOW
  pinMode(uecho, INPUT);                              // Set the ultrasonic echoPin as an INPUT

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
  
  Wire.begin();                                       //begin I2C connection to external EEPROM

  DFSoftwareSerial.begin(9600);                       //begin serial connection to MP3 player
  byte MP3setvol[] = {0x7E, 0xFF, 0x06, 0x06, 0x00, 0x00, EEPROM.read(volbyte), 0xEF};     //read stored volume level
  DFSoftwareSerial.write(MP3setvol, 8);                //set volume of MP3 player

  startupCh();    //run startupCh function to turn on channels that should be on in the untriggered state
  
  digitalWrite(led, HIGH); delay(100); digitalWrite(led, LOW); delay(100); digitalWrite(led, HIGH); delay(100); digitalWrite(led, LOW); delay(100); digitalWrite(led, HIGH); delay(100); digitalWrite(led, LOW); delay(100); digitalWrite(led, HIGH); delay(100); digitalWrite(led, LOW); delay(100); digitalWrite(led, HIGH); delay(100); digitalWrite(led, LOW); delay(100); digitalWrite(led, HIGH); delay(100); digitalWrite(led, LOW); delay(100); //blink six times for startup confirmation
}



void loop() {
  inputCheck();   //refresh the inputs

  
  if ((trigstate||sonicstate) & !shiftstate) {            //if triggered, play sequence
    digitalWrite(led, HIGH);  //turn on led for duration of playback
    playSeq();                //play sequence
  }
  else { digitalWrite(led, LOW);}    //turn of led when not playing back sequence
  

  if (ch1state && shiftstate){       //if shift and ch1 buttons are pressed, record sequence
    digitalWrite(led, HIGH); delay(600); digitalWrite(led, LOW); delay(600); digitalWrite(led, HIGH); delay(600); digitalWrite(led, LOW); delay(600); digitalWrite(led, HIGH); delay(600); digitalWrite(led, LOW);  //3 countdown blink
    recSeq();
  }


    if (ch2state && shiftstate){       //if shift and ch2 buttons are pressed, layer record sequence
    digitalWrite(led, HIGH); delay(600); digitalWrite(led, LOW); delay(600); digitalWrite(led, HIGH); delay(600); digitalWrite(led, LOW); delay(600); digitalWrite(led, HIGH); delay(600); digitalWrite(led, LOW);  //3 countdown blink
    recSeqLayer();
  }
  

  if (EEPROM.read(ambientbyte) && (analogRead(busy)>500)) {    //if play ambient track and ambient track has stoped
    playMP3(2);     //play 2nd MP3 (ambient)
    delay(25);
  }


  if (ch6state && shiftstate){     //toggle ambient enable eeprom bit if shift and ch6 are pressed
    if (EEPROM.read(ambientbyte) == 0){ EEPROM.write(ambientbyte, 1); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(1000); }
    else {EEPROM.write(ambientbyte, 0); digitalWrite(led, HIGH); delay(400); digitalWrite(led, LOW); delay(400); digitalWrite(led, HIGH); delay(400); digitalWrite(led, LOW); delay(400); digitalWrite(led, HIGH); delay(400); digitalWrite(led, LOW); delay(400); digitalWrite(led, HIGH); delay(400); digitalWrite(led, LOW); }
  }

  if (shiftstate  &&  trigstate)  {   //toggle ultrasonic enable eeprom bit if shift and trig are pressed
    if (EEPROM.read(sonicbyte) == 0){  EEPROM.write(sonicbyte, 1); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(1000); }
    else { EEPROM.write(sonicbyte, 0); digitalWrite(led, HIGH); delay(400); digitalWrite(led, LOW); delay(400); digitalWrite(led, HIGH); delay(400); digitalWrite(led, LOW); delay(400); digitalWrite(led, HIGH); delay(400); digitalWrite(led, LOW); delay(400); digitalWrite(led, HIGH); delay(400); digitalWrite(led, LOW); }
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



void playSeq(){
  unsigned long currentmemory = 0;
  playMP3(1);       //play 1st mp3 file

  unsigned long startmillis = millis();     //record start time
  bool keepGoin = HIGH;
  while (keepGoin) {
    byte instruct = readEEPROM(currentmemory); currentmemory++;     //read in next instruction and move to next memory position
    while ((millis()-startmillis) < ((currentmemory+1)*quant)){}        //wait to execute
    if (bitRead(instruct, 7) == 1) {                            //check to see if end bit has been reached
      keepGoin = LOW;   //if end bit reached, stop and exit loop 
    }
    updateCh(instruct);    //update outputs 
  }
}



void recSeq(){
  unsigned long currentmemory = 0;
  inputCheck();        //refresh inputs to clear shiftState
  playMP3(1);          //play 1st mp3 file

  unsigned long startmillis = millis();     //record start time

  while((!shiftstate)  && (currentmemory<65534)) {          //loop while user hasnt pressed shift button and there is still memory space left
    while((millis()-startmillis) < ((currentmemory+1)*quant)) {}       //wait to execute
    inputCheck();                                                      //refresh inputs
    byte chanStates = 0; chanStates|=ch1state<<0; chanStates|=ch2state<<1; chanStates|=ch3state<<2; chanStates|=ch4state<<3; chanStates|=ch5state<<4; chanStates|=ch6state<<5;    //convert input states to byte representation
    writeEEPROM(currentmemory, chanStates); currentmemory++;           //save byte
    updateCh(chanStates);                                              //update outputs  
  }
  byte StopBit = B10000000;
  delay(10);
  writeEEPROM(currentmemory, StopBit);       //write end stop bit (1 @ MSB)
  delay(10);
  digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); 
}



void recSeqLayer(){
  unsigned long currentmemoryL = 0;
  byte instructL = 0;      //
  playMP3(1);          //play 1st mp3 file

  unsigned long startmillisL = millis();     //record start time

  while(!(bitRead(instructL, 7) == 1)) {          //loop untill end of sequence
    while((millis()-startmillisL) < ((currentmemoryL+1)*quant)) {}
    instructL = readEEPROM(currentmemoryL);     //read in next instruction and move to next memory position    
    inputCheck();                               //refresh inputs
    byte chanStatesL = 0; chanStatesL|=ch1state<<0; chanStatesL|=ch2state<<1; chanStatesL|=ch3state<<2; chanStatesL|=ch4state<<3; chanStatesL|=ch5state<<4; chanStatesL|=ch6state<<5;    //convert input states to byte representation
    chanStatesL|=instructL;     //bitwise OR to create layering
    writeEEPROM(currentmemoryL, chanStatesL); currentmemoryL++;       //save combined byte
    updateCh(chanStatesL);              //update outputs
  }
  digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); 
}


void startupCh(){                                     //turns on channels that should be on in the untriggered state on controller startup 
  unsigned long currentmemory = 0;
  byte instructPrev = readEEPROM(currentmemory); currentmemory++;     //read in  instruction and move to next memory position
  byte instruct = readEEPROM(currentmemory); currentmemory++;     //read in next instruction and move to next memory position  
  while(bitRead(instruct, 7) != 1 && currentmemory<65534){    //while havent reched stop bit and is still memory
    instructPrev = instruct;
    instruct = readEEPROM(currentmemory); currentmemory++;     //read in next instruction and move to next memory position  
  }
  updateCh(instructPrev);
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



void writeEEPROM(unsigned int eeaddress, byte data ) {      //writes data to external EEPROM at eeaddress
  if (eeaddress <= 32767) {         //set EEPROM device address based on eeaddress
    Wire.beginTransmission(0x50);    //first EEPROM device
  }  
  else {
    eeaddress = eeaddress-32768;    //adjust address for second EEPROM device
    Wire.beginTransmission(0x51);   //second EEPROM device
  }

  Wire.write((int)(eeaddress >> 8));   // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.write((int) (data));         //send byte to be stored
  Wire.endTransmission();
  delay(1);
}

byte readEEPROM(unsigned int eeaddress ) {        //reads and returns external EEPROM byte at address eeaddress
  byte rdata = 0xFF;
  byte deviceaddress;
  if (eeaddress <= 32767) {     //set EEPROM device address based on eeaddress
    deviceaddress = 0x50;           //first EEPROM device
  }  
  else {
    eeaddress = eeaddress-32768;    //adjust address for second EEPROM device
    deviceaddress = 0x51;           //second EEPROM device
  }
 
  Wire.beginTransmission(deviceaddress);          //ask for byte from specific EEPROM chip
  Wire.write((int)(eeaddress >> 8));   // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.endTransmission();
 
  Wire.requestFrom(deviceaddress,1);              //get byte
  if (Wire.available()) rdata = Wire.read();
  return rdata;
}



void playMP3(unsigned int track) {        //plays MP3 tracks
  switch (track) {
    case 1:
          DFSoftwareSerial.write(playMP31, 8);      //send code to play 1st MP3
          break;
    case 2:
          DFSoftwareSerial.write(playMP32, 8);      //send code to play 2nd MP3
          delay(300);
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

  if (EEPROM.read(sonicbyte) == 1){        //if ultrasonic sensor is activated
    digitalWrite(utrig, HIGH);                       //trigger ultrasonic sensor
    delayMicroseconds(10);
    digitalWrite(utrig, LOW);
    long duration = pulseIn(uecho, HIGH, 12000);
    if (duration == 0) { duration=12000; }            //if no pulse is recieved (no ultrasonic conected/bad pulse) set to max length
    if (  map(constrain(duration, 150, 12000), 150, 12000, 1, 1024) < map(analogRead(pot), 0, 1024, 1024, 0)) {         //set sonicState HIGH if duration is less than potentiometer value
      sonicstate = HIGH;
    } else sonicstate = LOW;

    //ultrasonic sometimes gets falsly triggered when mp3 player stops playing ambient track
    if (EEPROM.read(ambientbyte)) {     //if play ambient track is enabled
      delay(15);                        //wait to allow busy pin to be analog read
      if (analogRead(busy)>500) {       //read busy pin to find out if ambient track has stoped
              sonicstate = LOW;         //set sonic state LOW to cancel any false triggers. 
      }
    }
  }
}
