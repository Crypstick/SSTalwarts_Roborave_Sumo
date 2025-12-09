#include "CytronMotorDriver.h"
#include <Ultrasonic.h>
#include <NewPing.h> // Include the NewPing library

// configure ultrasonic
Ultrasonic ultrasonic_front(10, 11); //first Trig pin, second Echo pin
Ultrasonic ultrasonic_left(8, 9); //first Trig pin, second Echo pin
Ultrasonic ultrasonic_right(12, 13); //first Trig pin, econd Echo pin

// configure ir ports
#define IR_LeftPin A0
#define IR_RightPin A1
#define IR_BackLeftPin A2
#define IR_BackRightPin A3

// Configure the motor driver.
CytronMD LeftMotor(PWM_DIR, 5, 4);  // PWM, DIR
CytronMD RightMotor(PWM_DIR, 6, 7); // PWM, DIR

void go_straight(int pwm) {
  LeftMotor.setSpeed(pwm);
  RightMotor.setSpeed(pwm);
}

NewPing sonar(10, 11, 200); // Trig, Echo, Max Distance
void setup() {
  pinMode(IR_LeftPin, INPUT);
  pinMode(IR_RightPin, INPUT);
  pinMode(IR_BackLeftPin, INPUT);
  pinMode(IR_BackRightPin, INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:

  // LeftMotor.setSpeed(128);
  // RightMotor.setSpeed(128);
  // delay(50);
  // LeftMotor.setSpeed(0);
  // RightMotor.setSpeed(0);
  // delay(50);
  // LeftMotor.setSpeed(-128);
  // RightMotor.setSpeed(-128);
  // delay(50);
  // LeftMotor.setSpeed(0);
  // RightMotor.setSpeed(0);
  // delay(50);

  // if (cm > 10 && cm < 20) {
  //   go_straight(200);
  // } else if (cm < 5) {
  //   go_straight(-200);
  // } else {
  //   go_straight(0);
  // }
  int center_ping = sonar.ping() / US_ROUNDTRIP_CM;
  LeftMotor.setSpeed(-200);
  RightMotor.setSpeed(200);
  delay(100);
  int left_ping = sonar.ping() / US_ROUNDTRIP_CM;
  LeftMotor.setSpeed(200);
  RightMotor.setSpeed(-200);
  delay(200);
  int right_ping = sonar.ping() / US_ROUNDTRIP_CM;
  LeftMotor.setSpeed(-200);
  RightMotor.setSpeed(200);
  delay(100);
  if (center_ping < left_ping && center_ping < right_ping && center_ping - left_ping > 5 || center_ping - right_ping > 5) {
    go_straight(0);
    delay(100);
  } else if (left_ping > right_ping && left_ping - right_ping > 5) {
    LeftMotor.setSpeed(-200);
    RightMotor.setSpeed(200);
    delay(100);
  } else if (right_ping - left_ping > 5) {
    LeftMotor.setSpeed(200);
    RightMotor.setSpeed(-200);
    delay(100);
  }
}
