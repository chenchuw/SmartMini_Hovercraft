/* This code serves for the BU ME461 Team 11's Smart Mini-Hovercraft senior design project.
It functions to remote control the hovercraft with FS-i6X and remote controller and FS-iA6B channel receiver.
It also let the hovercraft be able to use the ultrasonar sensor array to detect obstacles
and conduct certain actions when it detects obstacles. It also includes a RGB light to indicate
the safety level of current position. Written by Chuwei Chen | 2021 | chenchuw@bu.edu
Reference:
https://medium.com/@werneckpaiva/how-to-read-rc-receiver-signal-with-arduino-54e0447f6c3f
https://create.arduino.cc/projecthub/muhammad-aqib/arduino-rgb-led-tutorial-fc003e
*/

#include <Servo.h>

// Channel 1: Servo (Not implemented here), Channel 2: Propeller, Channel 3: Lifter
#define CH2 5 // Propeller
#define CH3 6 // Lifter

const int Trigger_Front = 11; // Trigger Pin of Ultrasonic Sensor
const int Echo_Front = 8; // Echo Pin of Ultrasonic Sensor
const int Trigger_Left =7;
const int Echo_Left = 4;
const int Trigger_Right = 3;
const int Echo_Right = 2;

int red_light_pin= 10;
int green_light_pin = 12;
int blue_light_pin = 0;

Servo ESC_propeller;
Servo ESC_lifter; // Under construction!

// Read the number of a given channel and convert to the range provided.
// If the channel is off, return the default value
int readChannel(int channelInput, int minLimit, int maxLimit, int defaultValue){
  int ch = pulseIn(channelInput, HIGH, 30000);
  if (ch < 100) return defaultValue;
  return map(ch, 1000, 2000, minLimit, maxLimit);
}

void setup(){
  Serial.begin(115200);
  ESC_propeller.attach(9,1000,2000);
  ESC_lifter.attach(13,1000,2000);
  pinMode(CH2, INPUT);
  pinMode(CH3, INPUT);
}

int ch2Value, ch3Value, ch2speed, ch3speed;

void loop() {
  // Sensors
  long duration_front, cm_front, duration_left, cm_left, duration_right, cm_right;
   // Front sensor
   pinMode(Trigger_Front, OUTPUT);
   digitalWrite(Trigger_Front, LOW);
   delayMicroseconds(2);
   digitalWrite(Trigger_Front, HIGH);
   delayMicroseconds(10);
   digitalWrite(Trigger_Front, LOW);
   pinMode(Echo_Front, INPUT);
   duration_front = pulseIn(Echo_Front, HIGH);
   cm_front = ms_to_cm(duration_front);

   // Left sensor
   pinMode(Trigger_Left, OUTPUT);
   digitalWrite(Trigger_Left, LOW);
   delayMicroseconds(2);
   digitalWrite(Trigger_Left, HIGH);
   delayMicroseconds(10);
   digitalWrite(Trigger_Left, LOW);
   pinMode(Echo_Left, INPUT);
   duration_left = pulseIn(Echo_Left, HIGH);
   cm_left = ms_to_cm(duration_left);

   // Right sensor
   pinMode(Trigger_Right, OUTPUT);
   digitalWrite(Trigger_Right, LOW);
   delayMicroseconds(2);
   digitalWrite(Trigger_Right, HIGH);
   delayMicroseconds(10);
   digitalWrite(Trigger_Right, LOW);
   pinMode(Echo_Right, INPUT);
   duration_right = pulseIn(Echo_Right, HIGH);
   cm_right = ms_to_cm(duration_right);
   
   Serial.print("Front sensor: ");
   Serial.print(cm_front);
   Serial.print("cm     ");
   Serial.print("Left sensor: ");
   Serial.print(cm_left);
   Serial.print("cm     ");
   Serial.print("Right sensor: ");
   Serial.print(cm_right);
   Serial.print("cm     ");

  // ESCs
  ch2Value = readChannel(CH2, 0, 100, 0);
  ch3Value = readChannel(CH3, 0, 100, 0);
//  if (ch2Value < 50)
//    ch2Value = 50;
  ch2speed = map(ch2Value, 50, 100, 1000, 2000); // 50(middle joystick), 100 (top joystick), 1000 (0 speed), 2000 (max speed)
  ch3speed = map(ch3Value, 0, 100, 1000, 2000); // 0(bottom joystick), 100(top joystick).
  
  // Important variables here: cm_front,cm_left,cm_right, ESC_lifter, ESC_propeller

  // Determine current situation: var=1 all green, var=2 one or more blue, var=3 one or more red.
  // Green(safe): nothing ahead, Blue(careful): something in between 20-30cm, Red(warning): something in 20cm.
  int varr = 0;
  // Nothing around:
  if (cm_front >= 30 and cm_left >= 30 and cm_right >= 30) varr = 1;
  // Be careful:
  else if (cm_front >= 20 && cm_left >= 20 && cm_right >= 20 && cm_front < 30) varr = 2; 
  else if (cm_front >= 20 && cm_left >= 20 && cm_right >= 20 && cm_left < 30) varr = 2; 
  else if (cm_front >= 20 && cm_left >= 20 && cm_right >= 20 && cm_right < 30) varr = 2; 
  else if (cm_front >= 20 && cm_left >= 20 && cm_right >= 20 && cm_front < 30 && cm_left < 30) varr = 2;
  else if (cm_front >= 20 && cm_left >= 20 && cm_right >= 20 && cm_front < 30 && cm_right < 30) varr = 2; 
  else if (cm_front >= 20 && cm_left >= 20 && cm_right >= 20 && cm_right < 30 && cm_left < 30) varr = 2; 
  else if (cm_front >= 20 && cm_left >= 20 && cm_right >= 20 && cm_right < 30 && cm_left < 30 && cm_front < 30) varr = 2; 
  // Too close:
  else varr = 3;
  
  
  // Obstacle-detected Decision Making && Remote Control
  // Green: do nothing, Blue: do nothing, Red: brake and delay 1s for user lowering speed.
  switch (varr) {
  case 1:
    RGB_color(0,255,0);
    ESC_lifter.write(ch3speed);
    if(ch2speed > 990)
      ESC_propeller.write(ch2speed);
    break;
    
  case 2:
    RGB_color(0,0,255);
    ESC_lifter.write(ch3speed);
    if(ch2speed > 990)
      ESC_propeller.write(ch2speed);
    break;
    
   case 3:
    RGB_color(255,0,0);
    ESC_lifter.write(1000);
    if(ch2speed > 990)
      ESC_propeller.write(1000);
    delay(1000); // Delay 1second for user to lower the speed
    break;
    
    default:
    ESC_lifter.write(ch3speed);
    if(ch2speed > 990)
      ESC_propeller.write(ch2speed);
    break;
  }

  Serial.print((int)varr);
  Serial.print(" Ch2: ");
  Serial.print(ch2Value);
  Serial.print(" Propeller Speed: ");
  Serial.print(ch2speed);
  Serial.print(" Ch3: ");
  Serial.print(ch3Value);
  Serial.print(" Lifter Speed: ");
  Serial.print(ch3speed);
  Serial.print('\n');
  delay(100);
}

long ms_to_cm(long microseconds) {
   return microseconds / 29 / 2;
}

void RGB_color(int red, int green, int blue)
 {
  analogWrite(red_light_pin, red);
  analogWrite(green_light_pin, green);
  analogWrite(blue_light_pin, blue);
}
