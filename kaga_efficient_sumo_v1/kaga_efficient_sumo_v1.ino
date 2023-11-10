/*******************************************************************************
   THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTY AND SUPPORT
   IS APPLICABLE TO THIS SOFTWARE IN ANY FORM. CYTRON TECHNOLOGIES SHALL NOT,
   IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR CONSEQUENTIAL
   DAMAGES, FOR ANY REASON WHATSOEVER.
 ********************************************************************************
   DESCRIPTION:

   This example shows how to drive 2 motors using the PWM and DIR pins with
   2-channel motor driver.


   CONNECTIONS:

   Arduino D3  - Motor Driver PWM 1 Input
   Arduino D4  - Motor Driver DIR 1 Input
   Arduino D9  - Motor Driver PWM 2 Input
   Arduino D10 - Motor Driver DIR 2 Input
   Arduino GND - Motor Driver GND


   AUTHOR   : Kong Wai Weng
   COMPANY  : Cytron Technologies Sdn Bhd
   WEBSITE  : www.cytron.io
   EMAIL    : support@cytron.io

 *******************************************************************************/

#include "CytronMotorDriver.h"
#include "Ultrasonic.h"

#define front 0
#define left 1
#define right 2
#define back 3

#define IR_VALUES 1
#define ULTRASONIC_VALUES 2
#define STAY_IN_CIRCLE_TESTING 3
#define SEARCH_AND_DESTROY_TESTING 4
#define FULL_GAME_LOOP 5
//---------------------------------config--------------------------------------------------------------------------

// configure ultrasonic
Ultrasonic US_front(10, 11); //first Trig pin, second Echo pin
Ultrasonic US_left(8, 9); //first Trig pin, second Echo pin
Ultrasonic US_right(12, 13); //first Trig pin, econd Echo pin

// configure ir ports
#define IR_LeftPin A0
#define IR_RightPin A1
#define IR_BackLeftPin A2
#define IR_BackRightPin A3
//---------------------------------config-----------------------------------------------------------------------------

// Configure the motor driver.
CytronMD leftMotor(PWM_DIR, 5, 4);  // PWM, DIR
CytronMD rightMotor(PWM_DIR, 6, 7); // PWM, DIR

// var set up
int isObstacleLeft = HIGH;
int isObstacleRight = HIGH;
int isObstacleLeftBack = HIGH;
int isObstacleRightBack = HIGH;



int last_value = 10; //last place where the opponnent was seen

float distance_front;
float distance_left;
float distance_right;
float nearest;

// qol functions
void rushForward() {
  leftMotor.setSpeed(255);
  rightMotor.setSpeed(255);
}
void moveForward() {
  leftMotor.setSpeed(140);
  rightMotor.setSpeed(140);
}
void moveBackward() {
  leftMotor.setSpeed(-140);
  rightMotor.setSpeed(-140);
}
void turnLeft() {
  leftMotor.setSpeed(-180);
  rightMotor.setSpeed(180);
}
void turnRight() {

  leftMotor.setSpeed(180);
  rightMotor.setSpeed(-180);
}


void foundTheOpponent(int dir) {
  Serial.println("found the opponent" + String(dir));
  if (dir == front) {
    if (last_value == left) {
      turnRight();
      delay(30);
    }
    else if (last_value == right) {
      turnLeft();
      delay(30);
    }
    if (distance_front < 10) {
      rushForward();
    } else {
      moveForward();
    }
    last_value = front;
  } else if (dir == left) {
    turnLeft();
    last_value = left;
  } else if (dir == right) {
    turnRight();
    last_value = right;
  }
}


// The setup routine runs once when you press reset.
void setup() {
  pinMode(IR_LeftPin, INPUT);
  pinMode(IR_RightPin, INPUT);
  pinMode(IR_BackLeftPin, INPUT);
  pinMode(IR_BackRightPin, INPUT);
  Serial.begin(9600);
  delay(1500); // tune as required
}

long tmp;
// The loop routine runs over and over again forever.
void loop() {
  // tmp = millis();
  
  isObstacleLeft = digitalRead(IR_LeftPin);
  isObstacleRight = digitalRead(IR_RightPin);
  isObstacleLeftBack = digitalRead(IR_BackLeftPin);
  isObstacleRightBack = digitalRead(IR_BackRightPin);

  //check for line
  if (isObstacleLeft == HIGH or isObstacleRight == HIGH) {
    leftMotor.setSpeed(-180);
    rightMotor.setSpeed(-180);
    // hopefully fixes the issue of the bot being completely locked on bot and suiciding
    delay(300);
    // turning right
    leftMotor.setSpeed(180);
    rightMotor.setSpeed(-180);
    delay(150);

  } else if (isObstacleLeftBack == HIGH or isObstacleRightBack == HIGH) {
    leftMotor.setSpeed(180);
    rightMotor.setSpeed(180);
    delay(150);
  

    // check for bot infront of sensors
  } else {
    distance_front = US_front.read();
    delay(3);
    distance_left = US_left.read();
    delay(3);
    distance_right = US_right.read();
    delay(3);

    
    if (distance_front < 75 && distance_front > 0) {
      foundTheOpponent(front);
    } else if (distance_left < 75 && distance_left > 0) {
      foundTheOpponent(left);
    } else if (distance_right < 75 && distance_right > 0) {
      foundTheOpponent(right);
     

    /*
    tmp = min(distance_front, disatance_left);
    nearest = min(tmp,  distance_right);
    if (nearest > 75 && nearest > 0) {
      if (nearest == distance_front) {
        foundTheOpponent(front);
      } else if (nearest == distance_left) {
        foundTheOpponent(left);
      } else if (nearest == distance_right) {
        foundTheOpponent(right);
      }
      */
    
      // check for last value fallback
    } else {
      if (last_value != 10) {
        foundTheOpponent(last_value);

        //just turn to the right
      } else {
        turnRight();
      }
    }
  }

  /*
  Serial.println(millis()-tmp);
  delay(10);
  */
}



















//reference
/*
  leftMotor.setSpeed(-255);   // Motor 1 runs forward at full speed.
  rightMotor.setSpeed(-255);  // Motor 2 runs backward at full speed.
  delay(1000);

  leftMotor.setSpeed(150);   // Motor 1 runs forward at full speed.
  rightMotor.setSpeed(150);  // Motor 2 runs backward at full speed.
  delay(1000);
*/
