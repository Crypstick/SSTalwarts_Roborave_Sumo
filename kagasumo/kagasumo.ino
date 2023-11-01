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

#include <arduino-timer.h>
auto timer = timer_create_default();

#define front 0
#define left 1
#define right 2
#define leftBack 3
#define rightBack 4

#define IR_VALUES 1
#define ULTRASONIC_VALUES 2
#define STAY_IN_CIRCLE_TESTING 3
#define SEARCH_AND_DESTROY_TESTING 4
#define FULL_GAME_LOOP 5
//---------------------------------config--------------------------------------------------------------------------

// configure ultrasonic
Ultrasonic ultrasonic_front(10, 11); //first Trig pin, second Echo pin
Ultrasonic ultrasonic_left(8, 9); //first Trig pin, second Echo pin
Ultrasonic ultrasonic_right(12, 13); //first Trig pin, econd Echo pin

// configure ir ports
#define IR_LeftPin A0
#define IR_RightPin A1
#define IR_BackLeftPin A2
#define IR_BackRightPin A3

// choose mode
int mode = FULL_GAME_LOOP;
  //IR_VALUES                   --> prints to serial port ir readings
  //ULTRASONIC_VALUES           --> prints to serial port ultrasonic readings
  //STAY_IN_CIRCLE_TESTING      --> runs the checking for line and going away from line without searching for opponet
  //SEARCH_AND_DESTROY_TESTING  --> runs the search for bot and then attack without searching for line
  //FULL_GAME_LOOP              --> full game loop with stay-in-cirlce + search-and-destroy tgt

//---------------------------------config-----------------------------------------------------------------------------

// Configure the motor driver.
CytronMD LeftMotor(PWM_DIR, 5, 4);  // PWM, DIR
CytronMD RightMotor(PWM_DIR, 6, 7); // PWM, DIR

// var set up
int isObstacleLeft = HIGH;
int isObstacleRight = HIGH;
int isObstacleLeftBack = HIGH;
int isObstacleRightBack = HIGH;

int last_value = front; //last place where the opponnent was seen

float distance_front;
float distance_left;
float distance_right;

bool tmp = true;

int iteration = 0;
int last_line_seen;

// qol functions
void rushForward() {
  LeftMotor.setSpeed(225);
  RightMotor.setSpeed(225);
}
void moveForward() {
  LeftMotor.setSpeed(138);
  RightMotor.setSpeed(138);
}
void moveBackward() {
  LeftMotor.setSpeed(-225);
  RightMotor.setSpeed(-225);
}
void moveBackwardForIR() {
  LeftMotor.setSpeed(-225);
  RightMotor.setSpeed(-225);
  if (isObstacleLeftBack == HIGH) {
      seen_the_line(leftBack);
    } else if (isObstacleRightBack == HIGH) {
      seen_the_line(rightBack);
}
void turnLeft() {
  LeftMotor.setSpeed(-160);
  RightMotor.setSpeed(160);
}
void turnRight() {
  LeftMotor.setSpeed(160);
  RightMotor.setSpeed(-160);
}
void stopMoving() {
  LeftMotor.setSpeed(0);
  RightMotor.setSpeed(0);
}
void updateSensors() {
  // ultrasonic values given in CM
  distance_front = ultrasonic_front.read();
  //delay(100);
  distance_left = ultrasonic_left.read();
  //delay(100);
  distance_right = ultrasonic_right.read();
  //delay(100);

  // IR sensor values
  isObstacleLeft = digitalRead(IR_LeftPin);
  isObstacleRight = digitalRead(IR_RightPin);
  isObstacleLeftBack = digitalRead(IR_BackLeftPin);
  isObstacleRightBack = digitalRead(IR_BackRightPin);
}
void foundTheOpponent(int dir) {
  if (dir == front) {
    if (last_value == left) {turnRight(); delay(15);}
    else if (last_value == right) {turnLeft(); delay(15);}
    if (distance_front < 20) {
      rushForward();
    } else {
      moveForward();
    }
    last_value = front;
  } else if (dir == left) {
    last_value = left;
    turnLeft();
  } else if (dir == right) {
    last_value = right;
    turnRight();
  }
}
void seen_the_line(int dir) {
  if (dir == left) {
    moveBackward();
    timer.in(500, turnRight);
    delay(250);
  } else if (dir == right) {
    moveBackward();
    delay(500);
    turnLeft();
    delay(200);
  } else if (dir == leftBack) {
    moveForward();
    delay(500);
    turnRight();
    delay(200);
  } else if (dir == rightBack) {
    moveForward();
    delay(500);
    turnLeft();
    delay(200);
  }
  moveForward();
}

// The setup routine runs once when you press reset.
void setup() {
  pinMode(IR_LeftPin, INPUT);
  pinMode(IR_RightPin, INPUT);
  pinMode(IR_BackLeftPin, INPUT);
  pinMode(IR_BackRightPin, INPUT);
  Serial.begin(9600);
}


// The loop routine runs over and over again forever.
void loop() {
  timer.tick();
  updateSensors();
  if (mode == FULL_GAME_LOOP) {

    //check for line
    if (isObstacleLeft == HIGH) {
      seen_the_line(left);
    } else if (isObstacleRight == HIGH) {
      seen_the_line(right);  
    } else if (isObstacleLeftBack == HIGH) {
      seen_the_line(leftBack);
    } else if (isObstacleRightBack == HIGH) {
      seen_the_line(rightBack);


    // check for bot infront of sensors
    } else { 
      if (distance_front < 50 and distance_front > 0) {
        foundTheOpponent(front);
      } else if (distance_left < 50 and distance_left > 0) {
        foundTheOpponent(left);
      } else if (distance_right < 50 and distance_right > 0) {
        foundTheOpponent(right);

      // check for last value fallback
      } else { 
        if (last_value != front) {
          foundTheOpponent(last_value);

        //just turn to the right
        } else { 
          turnRight();
        }
      }
    }
      
    




  } else if (mode == STAY_IN_CIRCLE_TESTING) {
        //check for line
    if (isObstacleLeft == HIGH) {
      seen_the_line(left);
    } else if (isObstacleRight == HIGH) {
      seen_the_line(right);  
    } else if (isObstacleLeftBack == HIGH) {
      seen_the_line(leftBack);
    } else if (isObstacleRightBack == HIGH) {
      seen_the_line(rightBack);
    }




  } else if (mode == SEARCH_AND_DESTROY_TESTING) {

    if (tmp) { //exit condition for when opponent is found
      if (distance_front < 50 and distance_front > 0 ) {
        Serial.println("FOUND OPPONENT IN FRONT BY: " + String(distance_front));
        foundTheOpponent(front);
      } else if (distance_left < 50) {
        Serial.println("FOUND OPPONENT IN LEFT BY: " + String(distance_left));
        foundTheOpponent(left);
      } else if (distance_right < 50) {
        Serial.println("FOUND OPPONENT IN RIGHT BY: " + String(distance_right));
        foundTheOpponent(right);
        
      } else if (last_value != front) { // use last value
          Serial.println("using last call value: " + String(last_value));
          foundTheOpponent(last_value);
      } else {
        Serial.println("no bot found :(");
        turnRight();
      }
    }



  } else if (mode == IR_VALUES) {

    Serial.println("\nIRs");
    Serial.println("left: " + String(digitalRead(IR_LeftPin)));
    Serial.println("right: " + String(digitalRead(IR_RightPin)));
    Serial.println("back left: " + String(digitalRead(IR_BackLeftPin)));
    Serial.println("back right: " + String(digitalRead(IR_BackRightPin)));
    delay(450);


  } else if (mode == ULTRASONIC_VALUES) {

    //Serial.println("\nUltrasonics");
    //Serial.println("front:" + String(distance_front));
    //Serial.println("left" + String(distance_left));
    Serial.println("right" + String(distance_right));
    delay(0);


  } else {
    Serial.println("no code chosen");
  }
  delay(10);
}



















//reference
/*
  LeftMotor.setSpeed(-255);   // Motor 1 runs forward at full speed.
  RightMotor.setSpeed(-255);  // Motor 2 runs backward at full speed.
  delay(1000);

  LeftMotor.setSpeed(150);   // Motor 1 runs forward at full speed.
  RightMotor.setSpeed(150);  // Motor 2 runs backward at full speed.
  delay(1000);
*/
