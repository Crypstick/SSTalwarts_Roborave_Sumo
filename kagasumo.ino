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
int mode = IR_VALUES;
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
int isObstacleBack = HIGH;

int last_value = front; //last place where the opponnent was seen

float distance_front;
float distance_left;
float distance_right;

bool tmp = true;

// qol functions
void moveForward() {
  LeftMotor.setSpeed(255);
  RightMotor.setSpeed(255);
}
void moveBackward() {
  LeftMotor.setSpeed(-255);
  RightMotor.setSpeed(-255);
}
void curveBackward(int dir) {
  if (dir == left) {
    LeftMotor.setSpeed(-255);
    RightMotor.setSpeed(-128);
  } else if (dir == right) {
    LeftMotor.setSpeed(-128);
    RightMotor.setSpeed(-255);
  }
}
void turnLeft() {
  LeftMotor.setSpeed(-200);
  RightMotor.setSpeed(2000);
}
void turnRight() {
  LeftMotor.setSpeed(200);
  RightMotor.setSpeed(-200);
}
void stopMoving() {
  LeftMotor.setSpeed(0);
  RightMotor.setSpeed(0);
}
void updateSensors() {
  // ultrasonic values given in CM
  distance_front = ultrasonic_front.read();
  distance_left = ultrasonic_left.read();
  distance_right = ultrasonic_right.read();

  // IR sensor values
  isObstacleLeft = digitalRead(IR_LeftPin);
  isObstacleRight = digitalRead(IR_RightPin);
  isObstacleBack = digitalRead(IR_BackPin);
}
void foundTheOpponent(int dir) {
  if (dir == front) {
    delay(0);
    if (last_value == left) {turnRight(); delay(30);}
    else if (last_value == right) {turnLeft(); delay(30);}
    moveForward();
    last_value = front;
  } else if (dir == left) {
    last_value = left;
    turnLeft();
  } else if (dir == right) {
    last_value = right;
    turnRight();
  }
}

// The setup routine runs once when you press reset.
void setup() {
  pinMode(IR_LeftPin, INPUT);
  pinMode(IR_RightPin, INPUT);
  pinMode(IR_BackPin, INPUT);
  Serial.begin(9600);

}


// The loop routine runs over and over again forever.
void loop() {
  updateSensors();
  if (mode == FULL_GAME_LOOP) {

    if (isObstacleLeft == LOW or isObstacleRight == LOW) { // check for line???
      Serial.println("Line in front");
      moveBackward();
      delay(900);

    } else { // check for bot infront of sensors
      if (distance_front < 50 and distance_front > 0) {
        Serial.println("FOUND OPPONENT IN FRONT BY: " + String(distance_front) );
        foundTheOpponent(front);
      } else if (distance_left < 50) {
        Serial.println("FOUND OPPONENT IN LEFT BY: " + String(distance_left));
        foundTheOpponent(left);
      } else if (distance_right < 50) {
        Serial.println("FOUND OPPONENT IN RIGHT BY: " + String(distance_right));
        foundTheOpponent(right);

      } else { // check for last value fallback
        if (last_value != front) {
          Serial.println("using last call value: " + String(last_value));
          foundTheOpponent(last_value);

        } else { //just turn to the right
          Serial.println("no bot found :(");
          turnRight();
        }
      }
    }




  } else if (mode == STAY_IN_CIRCLE_TESTING) {

    if (isObstacleLeft == HIGH or isObstacleRight == HIGH) { // check for line???
      Serial.println("Line in front");
      moveBackward();
      delay(900);
      turnRight();
      delay(100);
    } else {
      moveForward();
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
    Serial.println("left:" + String(digitalRead(IR_LeftPin)));
    Serial.println("right" + String(digitalRead(IR_RightPin)));
    Serial.println("back" + String(digitalRead(IR_BackPin)));
    delay(450);


  } else if (mode == ULTRASONIC_VALUES) {

    Serial.println("\nUltrasonics");
    Serial.println("front:" + String(ultrasonic_front.read()));
    Serial.println("left" + String(ultrasonic_left.read()));
    Serial.println("right" + String(ultrasonic_right.read()));
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
