#include <Arduino.h>
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
CytronMD leftMotor(PWM_DIR, 9, 8);
CytronMD rightMotor(PWM_DIR, 11, 13);

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

const int SAMPLE_SIZE = 20;
const int SENSOR_NUM = 5;
const int SENSOR_PIN[SENSOR_NUM] = {A0, A1, A2, A3, A4}; // A0 Clear, A1 Clear, A2 Clear, A3 Clear, A4 Clear
const int frontSensors[SENSOR_NUM] = {};

int sensor_readings[SENSOR_NUM][SAMPLE_SIZE];
int readings_it[SENSOR_NUM];
bool nums_filled[SENSOR_NUM];



//function declearc
int filter_sensor(int sensor);
int raw_sensor(int sensor);
int findMax(int nums[SAMPLE_SIZE]);
int findMin(int nums[SAMPLE_SIZE]);
int findMean(int nums[SAMPLE_SIZE]);
void debugging(int mode);

void setup() {
  Serial.begin(9600); //Enable the serial comunication
  pinMode(line_left, INPUT);
  pinMode(line_right, INPUT);
  // set initialison latch to false
  for (int i = 0; i < SENSOR_NUM; i++) {
    nums_filled[i] = false;
    readings_it[i] = 0;
  }
  delay(1500); // sumo rules, adjust as required. 
  //start code here
  
} 

void readSensors(int* distance_left, int* distance_frontLeft, int* distance_frontMiddle, int* distance_frontRight, int* distance_right, int* minFront) {
  *distance_left = filter_sensor(0);
  *distance_frontLeft = filter_sensor(1);
  *distance_frontMiddle = filter_sensor(2);
  *distance_frontRight = filter_sensor(3);
  *distance_right = filter_sensor(4);


  int values[] = {distance_frontMiddle, distance_frontLeft, distance_frontRight};
  *minFront = findMin(values);
};


void loop() {
  // put your main code here, to run repeatedly:

  // change pin numbers as required
  int distance_left;
  int distance_frontLeft;
  int distance_frontMiddle;
  int distance_frontRight;
  int distance_right;
  int minFront;
  readSensors(&distance_left, &distance_frontLeft, &distance_frontMiddle, &distance_frontRight, &distance_right, &minFront);



  bool isOpponentFront = (minFront < 75);
  if (isOpponentFront) {
      if (minFront == distance_frontMiddle) {
        Serial.println("hi middle");
        Serial.println(distance_frontMiddle);
        leftMotor.setSpeed(255);
        rightMotor.setSpeed(255);
      }
      else if (minFront == distance_frontLeft) {
        Serial.println("hi front lefty");
        Serial.println(distance_frontLeft);
        while (!(distance_frontMiddle < 75)) {
        readSensors(&distance_left, &distance_frontLeft, &distance_frontMiddle, &distance_frontRight, &distance_right, &minFront);
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
        Serial.println(distance_frontRight);
        Serial.println(distance_frontMiddle);
        while (!(distance_frontMiddle < 75)) {
          readSensors(&distance_left, &distance_frontLeft, &distance_frontMiddle, &distance_frontRight, &distance_right, &minFront);
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
      Serial.println("rotating");
      };
    };
    delay(5);
};

// put function definitions here:
void line_detection() {
  int lineDetect_left = digitalRead(line_left);
  int lineDetect_right = digitalRead(line_right);

  if (lineDetect_left == HIGH and lineDetect_right == HIGH) { //
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

int filter_sensor(int sensor) {
  int distance = raw_sensor(sensor);
  
  //initialising at program start
  if (!nums_filled[sensor]) {
    nums_filled[sensor] = true;
    for (int i = 0; i < SAMPLE_SIZE - 1; i++) {
      if (sensor_readings[sensor][i] == 0) {
        sensor_readings[sensor][i] = distance;
        nums_filled[sensor] = false;
        break;
      }
    }
    //is it the last slot left to fill
    if (nums_filled) sensor_readings[sensor][SAMPLE_SIZE - 1] = distance;

  }
  else { // main loop after initisalsting
    //replace old
    sensor_readings[sensor][readings_it[sensor]] = distance;
    if (readings_it[sensor] == SAMPLE_SIZE - 1) readings_it[sensor] = 0;
    else readings_it[sensor]++;
    
    

    //filter
    return findMean(sensor_readings[sensor]);
  }
}

int raw_sensor(int sensor) {
  // the last number makes the thing curved eg higher means when its closer the number changes slower, when further number changes faster
  //distance = 29.988 * pow(map(analogRead(A1), 0, 1023, 0, 5000)/1000.0, -1.173) ;
  int distance = 29.988 * pow(map(analogRead(SENSOR_PIN[sensor]), 0, 1023, 0, 5000) / 1000.0, -1.173) ;

  return distance;
}

int findMax(int nums[SAMPLE_SIZE]) {
  int highest;
  for (int i = 0; i < SAMPLE_SIZE; i++) {
    if (nums[i] > highest) {
      highest = nums[i];
    }
  }
  return highest;
}

int findMin(int nums[SAMPLE_SIZE]) {
  int lowest = 5000;
  for (int i = 0; i < SAMPLE_SIZE; i++) {
    if (nums[i] < lowest) {
      lowest = nums[i];
    }
  }
  return lowest;
}

int findMean(int nums[SAMPLE_SIZE]) {
  int sum = 0;
  for (int i = 0; i < SAMPLE_SIZE; i++) {
    sum += nums[i];
  }
  if (sum / SAMPLE_SIZE < 5) {
    return 1000;
  } else {
    return sum / SAMPLE_SIZE;
  }
}

