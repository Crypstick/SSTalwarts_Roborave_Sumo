#include <Arduino.h>
#include <iostream>
#include <algorithm>
#include "CytronMotorDriver.h"
#include "Timer.h"
#include "SharpIR.h"


using namespace std;

void line_detection();


// potentially use last value again???
#define front_left 0
#define front_middle 1
#define front_right 2
#define left 3
#define right 4


//--- configuration --- //

// motor driver
CytronMD leftMotor(PWM_DIR, 5, 4);
CytronMD rightMotor(PWM_DIR, 6, 7);

// Delay testing
Timer timer;

// Sharp IR sensors
SharpIR IR_frontLeft(SharpIR::GP2Y0A21YK0F, A0 );
SharpIR IR_frontMiddle(SharpIR::GP2Y0A21YK0F, A1 );
SharpIR IR_frontRight(SharpIR::GP2Y0A21YK0F, A2 );
SharpIR IR_left(SharpIR::GP2Y0A21YK0F, A3 );
SharpIR IR_right(SharpIR::GP2Y0A21YK0F, A4 );

// dohyo line sensors
#define line_left 7 
#define line_right 8


// variable declaration
int firstStart = HIGH;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(line_left, INPUT);
  pinMode(line_right, INPUT);
  delay(1500); // sumo rules, adjust as required. 
}

void loop() {
  // put your main code here, to run repeatedly:
  line_detection();

  int distance_frontLeft = IR_frontLeft.getDistance();
  int distance_frontMiddle = IR_frontMiddle.getDistance();
  int distance_frontRight = IR_frontRight.getDistance();
  int distance_left = IR_left.getDistance();
  int distance_right = IR_right.getDistance();

  int minFront = std::min({distance_frontMiddle, distance_frontLeft, distance_frontRight});

  bool isOpponentFront = (minFront < 75);

  if (isOpponentFront) {
      if (minFront == distance_frontMiddle) {
        Serial.println("hi middle");
        leftMotor.setSpeed(255);
        rightMotor.setSpeed(255);
      }
      else if (minFront == distance_frontLeft) {
        Serial.println("hi front lefty");
        while (!(distance_frontMiddle < 75)) {
        leftMotor.setSpeed(-200 + distance_frontLeft); // adjusts speed of rotation based on distance. if far away, curve is smoother. if close, curve is steeper
        rightMotor.setSpeed(255);
        }

        /*
        note: while loop could be an issue, big blocking code and if you dont track well, its suicide cos cld end up running it and stuck within while without line detection...

        last_value = front_left
        if (last_value = front_left) {
          // run above code?
        */
        }
      else {
        Serial.println("hi front righty");
        while (!(distance_frontMiddle < 75)) {
        leftMotor.setSpeed(255);
        rightMotor.setSpeed(-200 + distance_frontLeft); // adjusts speed of rotation based on distance. if far away, curve is smoother. if close, curve is steeper
        }
      }
    } else if (distance_left < 75 && distance_left < distance_right) {
      Serial.println("hi lefty");
    } else if (distance_right < 75 && distance_right < distance_left) {
      Serial.println("hi righty");
    } else {
      if (firstStart == HIGH) { //slight move forward if match js started and can't detect anywhere
        leftMotor.setSpeed(100);
        rightMotor.setSpeed(100); 
        firstStart = LOW;
      } else { // start rotating till detection happens
      leftMotor.setSpeed(-180); 
      rightMotor.setSpeed(180);
      };
    };
};

// put function definitions here:
void line_detection() {
  int lineDetect_left = digitalRead(line_left);
  int lineDetect_right = digitalRead(line_right);

  if (lineDetect_left == HIGH and lineDetect_right == HIGH) {
    leftMotor.setSpeed(-180);
    rightMotor.setSpeed(-180);
    delay(150);
  } else if (lineDetect_left == HIGH) {
    leftMotor.setSpeed(-180);
    rightMotor.setSpeed(-180);
    delay(150);
    // turn right
    leftMotor.setSpeed(180);
    rightMotor.setSpeed(-180);
    delay(100);
  } else {
    leftMotor.setSpeed(-180);
    rightMotor.setSpeed(-180);
    delay(150);
    // turn left
    leftMotor.setSpeed(-180); 
    rightMotor.setSpeed(180);
    delay(100);
  }
};
