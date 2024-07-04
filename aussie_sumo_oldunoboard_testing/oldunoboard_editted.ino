#include "CytronMotorDriver.h"
#include "Timer.h"

using namespace std;

// potentially use last value again???
#define left 0
#define front_left 1
#define front_middle 2
#define front_right 3
#define right 4


// variable declaration
//hardware consts
const int SENSOR_NUM = 5;
const int SENSOR_PIN[SENSOR_NUM] = {A0, A1, A2, A3, A4}; // A0 Clear, A1 Clear, A2 Clear, A3 Clear, A4 Clear
const int front_sensors_offset = 15; // threshold for when a bot is considered infront

const int line_left = 7;
const int line_right = 8;

CytronMD leftMotor(PWM_DIR, 9, 8);
CytronMD rightMotor(PWM_DIR, 11, 13);
const int speed = 255;

//sensing and filtering 
const int SAMPLE_SIZE = 20; //used by filter sensor
int sensor_readings[SENSOR_NUM][SAMPLE_SIZE]; //history of raw readings, updated by filter sensor
int readings_it[SENSOR_NUM]; //current iteratior of sensor_readings for each sensor, updated by filter sensor
bool nums_filled[SENSOR_NUM]; // have the indivisual sensors filled up their sensor_readings, updated by filter sensor

int sensor_values[SENSOR_NUM]; // current filtered values, edited by update all sensors
bool firstStart = true; //game loop
bool game_start = false; //game loop



 //function declearc
//sensor funs
int raw_sensor(int sensor);
int filter_sensor(int sensor);
void update_all_sensors();

//others
int findDirection(int lowest_it);
void setMotors(int left_speed, int right_speed);
int line_detection();
//void debugging(int mode); //todo

//math funcs
int findMax_it(int nums[], int size); //RETURNS THE ITERATOR OF MIN and max
int findMin_it(int nums[], int size);
int findMean(int nums[], int size);


void setup() {
  Serial.begin(9600); //Enable the serial comunication
  pinMode(line_left, INPUT);
  pinMode(line_right, INPUT);
  // set initialison latch to false
  for (int i = 0; i < SENSOR_NUM; i++) {
    nums_filled[i] = false;
    readings_it[i] = 0;
  }
  game_start = true;
} 



void loop() {
  // put your main code here, to run repeatedly:
  update_all_sensors();
  if (line_detection() == 0) {
    int lowest_it = findMin_it(sensor_values, SENSOR_NUM);
    int direction = findDirection(lowest_it);
    int min = sensor_values[lowest_it];
  
    if (firstStart) {
      if (game_start){
        //sumo rules
        delay(1500);
        setMotors(255, 255);
        delay(100);
        firstStart = false;
      }
    } 
    else if (min > 6 && min < 30) { // tune range as required
      if (direction == front_middle) setMotors(speed, speed);
      else if (direction == left || direction == front_left) setMotors(speed * -1, speed);
      else if (direction == front_right || direction == right) setMotors(speed, speed * -1);
    } 
    else {
      setMotors(-255,255);
    }
  }
}

// put function definitions here:
int line_detection() {
  int lineDetect_left = digitalRead(line_left);
  int lineDetect_right = digitalRead(line_right);

  if (lineDetect_left == HIGH and lineDetect_right == HIGH) { //
    leftMotor.setSpeed(-180);
    rightMotor.setSpeed(-180);
    delay(150);
    return 1;
  }
  if (lineDetect_left == HIGH) {
    leftMotor.setSpeed(-180);
    rightMotor.setSpeed(-180);
    delay(150);
    // turn right
    leftMotor.setSpeed(180);
    rightMotor.setSpeed(-180);
    delay(100);
    return 1;
  }
  if (lineDetect_right == HIGH){
    leftMotor.setSpeed(-180);
    rightMotor.setSpeed(-180);
    delay(150);
    // turn left
    leftMotor.setSpeed(-180); 
    rightMotor.setSpeed(180);
    delay(100);
    return 1;
  }
  return 0;
}

int findDirection(int lowest_it) {
  if (lowest_it == front_left) {
    if (abs(sensor_values[front_middle] - sensor_values[front_left]) < front_sensors_offset) return front_middle;
  }
  else if (lowest_it == front_right) {
    if (abs(sensor_values[front_middle] - sensor_values[front_right]) < front_sensors_offset) return front_middle;
  }
  else return lowest_it;

  /*
  for (int i = 0; i < 5; i++) {
    if (values[i] == min) {
      if ((i == 1 || i == 2 || i == 3) && (abs(values[2] - values[1]) < front_sensors_offset || abs(values[2] - values[3]) < front_sensors_offset)) {
        return 2;
      } else {
        return i;
      }
    }
  }
  */
}

//bot ir processing
void update_all_sensors(){
  for (int i = 0; i < SENSOR_NUM; i++) {
    sensor_values[i] = filter_sensor(SENSOR_PIN[i]);
  }
}

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
    return findMean(sensor_readings[sensor], SAMPLE_SIZE);
  }
}

int raw_sensor(int sensor) {
  // the last number makes the thing curved eg higher means when its closer the number changes slower, when further number changes faster
  //distance = 29.988 * pow(map(analogRead(A1), 0, 1023, 0, 5000)/1000.0, -1.173) ;
  int distance = 29.988 * pow(map(analogRead(SENSOR_PIN[sensor]), 0, 1023, 0, 5000) / 1000.0, -1.173) ;

  return distance;
}

void setMotors(int left_speed, int right_speed) {
  leftMotor.setSpeed(left_speed);
  rightMotor.setSpeed(right_speed);
}

//math
int findMax_it(int nums[], int size) {
  int highest = 0;
  int it;
  for (int i = 0; i < size; i++) {
    if (nums[i] > highest) {
      highest = nums[i];
      it = i;
    }
  }
  return it;
}

int findMin_it(int nums[], int size) {
  int lowest = 5000;
  int it;
  for (int i = 0; i < size; i++) {
    if (nums[i] < lowest) {
      lowest = nums[i];
      it = i;
    }
  }
  return it;
}

int findMean(int nums[], int size) {
  int sum = 0;
  for (int i = 0; i < size; i++) {
    sum += nums[i];
  }
  if (sum / size < 5) {
    return 1000;
  } else {
    return sum / size;
  }
}
