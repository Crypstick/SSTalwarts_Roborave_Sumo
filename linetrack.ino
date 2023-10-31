// C++ code
//
#include "CytronMotorDriver.h"

int distance;
bool spinning;

long readUltrasonicDistance(int triggerPin, int echoPin){
  pinMode(triggerPin, OUTPUT);  // Clear the trigger
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  // Sets the trigger pin to HIGH state for 10 microseconds
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);
  // Reads the echo pin, and returns the sound wave travel time in microseconds
  return pulseIn(echoPin, HIGH);
}

CytronMD Motor(PWM_DIR, 5, 4);

void setup()
{
  Serial.begin(9600);
  spinning = false;
}

void loop()
{
  distance = 0.01723 * readUltrasonicDistance(9, 10);
  Serial.println(distance);
  if (distance < 7 && spinning == false) {
    Serial.println("start spin");
    Motor.setSpeed(255);
    spinning = true;
  } else {
    if (distance > 9 && spinning == true) {
      Serial.println("stop spiin");
      Motor.setSpeed(0);
      spinning = false;
    }
  }
  delay(50); // Wait for 50 millisecond(s)
}
