#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
SoftwareSerial MP3Module(4, 5);
DFRobotDFPlayerMini MP3Player;
#define echoPin 2
#define trigPin 3 //sonic sensor pins
#define mode 10 // mode selection
#define sonicpin 6 // c
#define mpcont 8  // input to control mp3 on/off

long duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement
int adv = 3;
int save;
int count;

void setup () {
  
  Serial.begin (9600);
  MP3Module.begin(9600);
  if (!MP3Player.begin(MP3Module)) { //mp3 module check
    Serial.println(F("MP3 failed"));
    while (true);
    
    
  }
  delay(1);
  pinMode(trigPin, OUTPUT); // Set the trigPin as an output
  pinMode(echoPin, INPUT); // Set the echoPin as an input
  pinMode(mpcont, INPUT_PULLUP); // pin between boards

  pinMode(mode, INPUT_PULLUP);
  pinMode(sonicpin, OUTPUT);
  Serial.begin(9600); // baudrate speed

  MP3Player.volume(10);  //mp3 volume
  MP3Player.loop(1);


}

void loop () {
   
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);

  distance = duration * 0.034 / 2; 

  if(distance<800){
    save = save + distance;
    count = count + 1;
  }
  if(count>5){
    count = 0;
    save = 0;
  }
  Serial.print(distance);
  Serial.println(" cm");

  if(distance>15){ 
   MP3Player.stopAdvertise();
   adv = 3;  
   digitalWrite(sonicpin, LOW);   
 } 
 if(distance<15){
  if(adv==3){
   MP3Player.advertise(1);
   adv = 2;
  }
  digitalWrite(sonicpin, HIGH);
  }

 if(digitalRead(mpcont)==LOW){
   MP3Player.volume(0);
 }
 if(digitalRead(mpcont)==HIGH){
   MP3Player.volume(10);
 }
}
