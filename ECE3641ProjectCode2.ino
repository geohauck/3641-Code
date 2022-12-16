/*
This code was hacked together by George Hauck
on 12/9/2022 as an attempt to control a linear
actuator based on the recorded angle of an IMU.
Who knows if sensor drift will play a role in
whether this system works? I took most () of the 
IMU <3 communication code from this website:
https://how2electronics.com/measure-tilt-angle-mpu6050-arduino/

Note: The IMU reads 90 degrees when rotated clockwise
180 degrees when flat (pins up, holes down)
and 270 degrees when rotated counter-clockwise

Note 2: if FwTilt is HIGH, then the bucket is too far curled out
if BwTilt is High, then the bucket is too far curled in

Note 3: There are plenty of double-negative opportunities here.
So if my code doesn't make sense, I might just flip a couple
wires instead. Easier and faster.
*/

#include <Wire.h>

// GIVEN Variables for IMU------------------------------------------------------------------------

const int MPU_addr = 0x68;
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

int minVal = 265;
int maxVal = 402;

double x;
double y;
double z;

// MY Variables -------------------------------------------------------------------------------
int leverForwardPin = 8;
int leverBackwardPin = 9;

// int FwTilt;
// int BwTilt;

int deltaAngle = 20;
int zeroAngle = 180;  // With IMU in correct orientation, pins at top of board

int maxBackCurl;
int maxForCurl;

int leverForward = 0;
int leverBackward = 0;

int moveTime = 5110;  // extend/retract the linear actuator in 3 second intervals
//3.94 stick back, 
void setup() {
  Wire.begin();  // Setup the Arduino for IMU communication
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  Serial.begin(9600);  // Setup serial monitor

  // Setting Up my action threshold values
  maxForCurl = zeroAngle - deltaAngle;
  maxBackCurl = zeroAngle + deltaAngle;

  // Setting up my control pins
  pinMode(leverForwardPin, OUTPUT);  // declare the control pins as outputs
  pinMode(leverBackwardPin, OUTPUT);

  digitalWrite(leverForwardPin, LOW);  // initalize both pins to LOW
  digitalWrite(leverBackwardPin, LOW);
}

void loop() {

  getData();  // Find the bucket position
  printData();


  // Logic, round 2!
  if (z > maxForCurl && z < maxBackCurl) {  // if the bucket is level
    Serial.println("The bucket is level");
    if (leverForward == 1) {  // if the lever is pushed forward
      MoveLeverBackward();
      leverForward = 0;  // lever is no longer forward
      Serial.println("Lever has returned to neutral state from forward");
    }
    if (leverBackward == 1) {  // if the lever is pulled backward
      MoveLeverForward();
      leverBackward = 0;  // lever is no longer backward
      Serial.println("Lever has returned to neutral state from backward");
    }
  }
  if (leverForward == 0 && leverBackward == 0) {  // if neither lever is moved yet
    if (z < maxForCurl) {                         // if bucket is too far forward
      MoveLeverForward();                         // move the lever forward
      leverForward = 1; // the lever is moved forward
      Serial.println("Bucket is too far forward, the lever has been moved forward");
    }
    if (z > maxBackCurl) {  // if bucket is too far backwards
      MoveLeverBackward();  // move the lever backward      
      leverBackward = 1; // the lever is moved forward
      Serial.println("Bucket is too far backward, the lever has been moved backward");
    }
  }
}

void MoveLeverForward() {
  digitalWrite(leverForwardPin, HIGH);  // Power the linear actuator out
  delay(moveTime);
  digitalWrite(leverForwardPin, LOW);  // Stop moving the linear actuator
}

void MoveLeverBackward() {
  digitalWrite(leverBackwardPin, HIGH);  // Power the linear actuator out
  delay(moveTime);
  digitalWrite(leverBackwardPin, LOW);  // Stop moving the linear actuator
}

void getData() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true);
  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();
  int xAng = map(AcX, minVal, maxVal, -90, 90);
  int yAng = map(AcY, minVal, maxVal, -90, 90);
  int zAng = map(AcZ, minVal, maxVal, -90, 90);

  x = RAD_TO_DEG * (atan2(-yAng, -zAng) + PI);
  y = RAD_TO_DEG * (atan2(-xAng, -zAng) + PI);
  z = RAD_TO_DEG * (atan2(-yAng, -xAng) + PI);
}

void printData() {
  Serial.print("AngleX= ");
  Serial.print(x);

  Serial.print("\tAngleY= ");
  Serial.print(y);

  Serial.print("\tAngleZ= ");
  Serial.println(z);
  delay(200);
}
