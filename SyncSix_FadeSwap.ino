//SyncSix FadeSwap Arduino Sketch      
//Channels 1-3 are on by default. When the trigger is activated, channels 1-3 fade out while 4-6 fade in. 
//The duration of the fade is set via the ultrasonic distance potentiometer.
//The controller will keep channels 1-3 off and 4-6 on for a set amount of time before resetting. This time is set by holding SHIFT and tapping CH1/REC.
//The indicator led will count down 3 times and stay illuminated until SHIFT is pressed again. The time the indicator LED is lit is how long the delay until reset will be.


unsigned int fadeTime;

#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <Wire.h>  

#define ambientbyte 0     //define arduino nano internal EEPROM settings bytes
#define volbyte 1
#define sonicbyte 2
#define waitTimebyte 3    //waitTime byte to store how long to wait to reset

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
  pinMode(pot, INPUT);                                //setup potentiometer
  pinMode(busy, INPUT);                               //setup MP3 player feedback pin

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

  updateCh(0b00000111);

  digitalWrite(led, HIGH); delay(100); digitalWrite(led, LOW); delay(100); digitalWrite(led, HIGH); delay(100); digitalWrite(led, LOW); delay(100); digitalWrite(led, HIGH); delay(100); digitalWrite(led, LOW); delay(100); digitalWrite(led, HIGH); delay(100); digitalWrite(led, LOW); delay(100); digitalWrite(led, HIGH); delay(100); digitalWrite(led, LOW); delay(100); digitalWrite(led, HIGH); delay(100); digitalWrite(led, LOW); delay(100); //blink six times for startup confirmation
}



void loop() {
  inputCheck();   //refresh the inputs

  
  if ((trigstate||sonicstate) & !shiftstate) {            //if triggered, fade LEDs, then wait to reset
    digitalWrite(led, HIGH);  //turn on led for duration of playback
    playMP3(1);
    for (int i=0; i<=fadeTime; i++){      //fade LEDs with varying PWM duty cycle
      updateCh(0b00000111);
      delayMicroseconds(fadeTime-i);
      updateCh(0b00111000);
      delayMicroseconds(i);
    }
    unsigned long delayTime = EEPROM.read(waitTimebyte)*2000UL;
    delay(delayTime);
    updateCh(0b00000111);
  }
  else { digitalWrite(led, LOW);}    //turn off led when not playing back sequence
  


  if (ch1state && shiftstate){       //if shift and ch1 buttons are pressed, record delay time to reset after fade; waitTime
    digitalWrite(led, HIGH); delay(600); digitalWrite(led, LOW); delay(600); digitalWrite(led, HIGH); delay(600); digitalWrite(led, LOW); delay(600); digitalWrite(led, HIGH); delay(600); digitalWrite(led, LOW); delay(600); digitalWrite(led, HIGH);  //3 countdown blink
    inputCheck();   //refresh the inputs
    unsigned long pastSeconds = millis()/2000UL;
    while(!shiftstate){
      inputCheck();   //refresh the inputs
    }
    unsigned long currentSeconds = millis()/2000UL;
    digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); digitalWrite(led, HIGH); delay(50); digitalWrite(led, LOW); delay(50); 
    EEPROM.write(waitTimebyte, lowByte(currentSeconds-pastSeconds));
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
  } else trigstate = LOW;

  fadeTime = map(analogRead(pot), 0, 1024, 5000, 500);    //read ultrasonic distance pot and convert to fadeTime delay value to adjust time taken to fade
}
