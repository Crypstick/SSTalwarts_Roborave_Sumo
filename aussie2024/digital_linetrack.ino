
/*
 * todo
 * 1.try to get a coded runfi
 * 2. move buzzer to inttrurppt func and latch remoce func
 */

//#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>
#include <digitalWriteFast.h>
#include <CytronMotorDriver.h>

//debug modes
#define RAW_VALUES 1
#define PROCESSED_AND_CONSECUTIVES 2

#define PID_AND_MOTOR 1

#define JUNCTION 1
#define DEGREES 2
#define TURN_AT_JUNC 3
#define FIND_LINE 4

#define LEFT 1
#define RIGHT 2
#define NONE 0


//hardware defintions
CytronMD left_motor(PWM_DIR, 5, 4);  // PWM 1 = Pin 3, DIR 1 = Pin 4.
CytronMD right_motor(PWM_DIR, 6, 7); // PWM 2 = Pin 9, DIR 2 = Pin 10.
Encoder encoder(2, 3); // 1 rotation is arnd 345

const int motor_switch = 8;
const int buzzer = 11;

//---------------------------   SETTINGS DEFAULTS PAGE -----------------
const int NUM_SENSORS = 6;
//const int sensorPins[NUM_SENSORS] = {34, 36, 38, 40, 42, 44, 46, 48, 50, 52}; //left to right]
const int sensorPins[NUM_SENSORS] = {18, 40, 42, 44, 46, 19}; //left to right]

const int def_kP = 10;
const int def_kD = 9;
const int def_kI = 0;
const int RESET_LATCH_DISTANCE = 1.5;
int edge_protection = false;

const int def_juncThreshold = 2;

const int def_baseSpeed = 90;
const int def_maxSpeed = 255;
const int def_minSpeed = -255;
const float Left_Latch_Speed[2] = {-80, 100};
const float Right_Latch_Speed[2] = {100, -80};
//---------------------------   SETTINGS DEFAULTS PAGE -----------------

// control parameters
const float MIDDLE_ARR = (NUM_SENSORS + 1) / 2.0;
int kP, kD, kI, juncThreshold, baseSpeed, maxSpeed, minSpeed;
#define PASS -1000
#define PASS_ALL PASS, PASS, PASS, PASS, PASS, PASS, PASS

// functions
void process_input(int debug = 0); //debug --> RAW_VALUES, PROCESSED_AND_CONSECUTIVES
void pid_loop(int debug = 0); //debug --> PID_AND_MOTOR
void broadcast(int mode = 0); //mode --> JUNCTION, DEGREES

void Junction();
void Degrees(long amt);
void turnAtJunc(int dir);
void findTheLine();
void config_settings(int tmp_kP, int tmp_kD, int tmp_kI, int tmp_juncThreshold, int tmp_baseSpeed, int tmp_maxSpeed, int tmp_minSpeed);
void drivetrain_setSpeed(int left, int right);

void check_interrupt();
void left_edge_trig();
void right_edge_trig();

// input processing vars
int sensorReadings[NUM_SENSORS], highestConsec;
float pos;

//control vars
float error, pastError, result;
long sumError;

//drivetrain vars
float left_speed, right_speed;
volatile int latched = NONE;
bool motorActivated = true;















void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(motor_switch, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  drivetrain_setSpeed(0, 0);
  for (int i = 0; i < NUM_SENSORS; i++) { //set pin mode for line array pins
    pinMode(sensorPins[i], INPUT);
  }
  attachInterrupt(digitalPinToInterrupt(sensorPins[0]), left_edge_trig, RISING);
  attachInterrupt(digitalPinToInterrupt(sensorPins[NUM_SENSORS-1]), right_edge_trig, RISING);
  config_settings(PASS_ALL); // set up settings
  motorActivated = true;



  //run codez
  //kP, kD, kI, juncThreshold, baseSpeed, maxSpeed, minSpeed
  //PASS,PASS,PASS,PASS,PASS,PASS,PASS
  edge_protection = true;

  findTheLine();

  config_settings(10,9,PASS,PASS,90,PASS,PASS);
  Degrees(2050);
  config_settings(18,9,PASS,PASS,45,PASS,PASS);
  Junction();

  
  turnAtJunc(LEFT);
  
  config_settings(15,0,PASS,PASS,90,PASS,PASS);
  Degrees(1400);
  config_settings(18,9,PASS,PASS,45,PASS,PASS);
  Junction();

  drivetrain_setSpeed(90, 80);
  delay(300);
  turnAtJunc(RIGHT); 

  config_settings(15,0,PASS,PASS,PASS,PASS,PASS);
  Degrees(2000);

  drivetrain_setSpeed(100, 100);
  delay(1000);
 
  drivetrain_setSpeed(-90, -80);
  delay(300);
  turnAtJunc(RIGHT); 
  
  config_settings(15,0,PASS,PASS,PASS,PASS,PASS);
  Degrees(1600);

  config_settings(18,9,PASS,PASS,45,PASS,PASS);
  Junction();

  turnAtJunc(LEFT); 
  
  
}


void loop() {
  while (false){
    // put your main code here, to run repeatedly:
    if (digitalReadFast(motor_switch) == HIGH) motorActivated = false;
    else motorActivated = true;
    
    process_input();
    pid_loop();
    //check_interrupt();
    //Serial.println(latched);
    Serial.println(encoder.read());
  }                                               

}











void pid_loop(int debug) { // loop and control motor
  //check if edge case
  pastError = error;
  error = pos - MIDDLE_ARR;
  if (latched != NONE and edge_protection) { // unlatch if inner sensor hit
    analogWrite(buzzer, 50);
    if (latched == LEFT) {
      if (fabs(error) <= RESET_LATCH_DISTANCE and sensorReadings[0] == LOW) {
        latched = NONE;
        digitalWrite(buzzer, LOW);
        drivetrain_setSpeed(Left_Latch_Speed[0]*-0.5, Left_Latch_Speed[1]*-10.5);
        delay(5);
      }
      else drivetrain_setSpeed(Left_Latch_Speed[0], Left_Latch_Speed[1]);
    } 
    else if (latched == RIGHT) {
      if (fabs(error) <= RESET_LATCH_DISTANCE and sensorReadings[NUM_SENSORS - 1] == LOW) {
        latched = NONE;
        digitalWrite(buzzer, LOW);
        drivetrain_setSpeed(Right_Latch_Speed[0] *-0.5, Right_Latch_Speed[1]*-0.5);
        delay(5);
      }
      else drivetrain_setSpeed(Right_Latch_Speed[0], Right_Latch_Speed[1]);
    }
   
  }
  else {
 
    //if (fabs(error) < 1) error = 0;
    sumError += error;
    result = (error * kP) + ( (error - pastError) * kD ) + (sumError * kI);

    //implement base speed
    left_speed = baseSpeed + result;
    right_speed = baseSpeed - result;

    if (result > 0) right_speed -= result;
    else left_speed += result;
 


    //floor and ceiling the speed
    if (left_speed > maxSpeed) left_speed = maxSpeed;
    else if (left_speed < minSpeed) left_speed = minSpeed;
    if (right_speed > maxSpeed) left_speed = maxSpeed;
    else if (right_speed < minSpeed) left_speed = minSpeed;

    //set the speed
    drivetrain_setSpeed(left_speed, right_speed);
  }

  //debug
  // PID_AND_MOTOR --> prints the error, past error, sumerror + L and R motor speeds + position
  if (debug == PID_AND_MOTOR) {
    Serial.print("pos: " + String(pos));
    Serial.print(", Left Speed: " + String(left_speed));
    Serial.print(", Right Speed: " + String(right_speed));
    Serial.print(", error: " + String(error));
    Serial.print(", pastError: " + String(pastError));
    Serial.print(", sumError: " + String(sumError));
    Serial.println();
  }
}



void Degrees(long amt) {
  encoder.write(0); //reset encoder pos
  
  while (encoder.read() < amt) {
    if (digitalReadFast(motor_switch) == HIGH) motorActivated = false;
    else motorActivated = true;
    process_input();
    pid_loop();
  }
  broadcast(DEGREES);
}

void Junction() {
  while (juncThreshold > highestConsec) {
    if (digitalReadFast(motor_switch) == HIGH) motorActivated = false;
    else motorActivated = true;
    process_input();
    pid_loop();
  }
  broadcast(JUNCTION);
}

void broadcast(int mode) { //handles the signal of end of segment; maybe add a buzzer some day
  drivetrain_setSpeed(0, 0);
  digitalWrite(buzzer, HIGH);
  delay(100);
  digitalWrite(buzzer, LOW);
  if (mode == JUNCTION) {
    Serial.println("junction");
  }
  else if (mode == DEGREES) {
    Serial.println("degrees");
  }
  else if (mode == TURN_AT_JUNC){
    Serial.println("turn at junc");
  }
}

void turnAtJunc(int dir) {
  //ADD THIS INT EH SETUP
  //drivetrain_setSpeed(90, 80);    
  //delay(300);
  
  if (dir == LEFT) {
    drivetrain_setSpeed(-60, 60);
    while (digitalReadFast(sensorPins[2]) == HIGH) {}
    while (digitalReadFast(sensorPins[2]) == LOW) {}
  }
  else {
    drivetrain_setSpeed(60, -60);
    while (digitalReadFast(sensorPins[NUM_SENSORS-3]) == HIGH) {}
    while (digitalReadFast(sensorPins[NUM_SENSORS-3]) == LOW) {}
  }
  broadcast(TURN_AT_JUNC);
  
}

void findTheLine() {
  process_input();
  while (highestConsec < juncThreshold) {
    if (digitalReadFast(motor_switch) == LOW) drivetrain_setSpeed(50, 45);
    else drivetrain_setSpeed(0, 0);
    process_input();
  }
  delay(5);
}

void config_settings(int tmp_kP, int tmp_kD, int tmp_kI, int tmp_juncThreshold, int tmp_baseSpeed, int tmp_maxSpeed, int tmp_minSpeed) {
  if (tmp_kP == PASS) kP = def_kP;
  else kP = tmp_kP;
  if (tmp_kD == PASS) kD = def_kD;
  else kD = tmp_kD;
  if (tmp_kI == PASS) kI = def_kI;
  else kI = tmp_kI;
  if (tmp_juncThreshold == PASS) juncThreshold = def_juncThreshold;
  else juncThreshold = tmp_juncThreshold;
  if (tmp_baseSpeed == PASS) baseSpeed = def_baseSpeed;
  else baseSpeed = tmp_baseSpeed;
  if (tmp_maxSpeed == PASS) maxSpeed = def_maxSpeed;
  else maxSpeed = tmp_maxSpeed;
  if (tmp_minSpeed == PASS) minSpeed = def_minSpeed;
  else minSpeed = tmp_minSpeed;
}

void drivetrain_setSpeed(int left, int right) {
  left_motor.setSpeed(left*motorActivated);
  right_motor.setSpeed(right*motorActivated);
}

void process_input(int debug) {
  int curConsec, numFoundPos, posSum;
  curConsec = numFoundPos = posSum = highestConsec = 0;

  // we poll the entire array first and then compute later so that our dataset makes more sense and dont have timing issues
  for (int i = 0; i < NUM_SENSORS; i++) {
    sensorReadings[i] = digitalReadFast(sensorPins[i]);
  }

  for (int i = 0; i < NUM_SENSORS; i++) {
    if (sensorReadings[i] == HIGH) { //
      posSum += i + 1; //left most sensor is 1, right most sensor is 10
      numFoundPos += 1;
      curConsec += 1;
    }
    else {
      if (curConsec > highestConsec) highestConsec = curConsec; //finds the highest consecutive number of triggered sensors for use in junction detection
      curConsec = 0;
    }
  }
  if (curConsec > highestConsec) highestConsec = curConsec; //force above check in case last sensor isnt white

  if (numFoundPos != 0) { // if there is no line seen, retain the old value
    pos = posSum / numFoundPos; // find the average if multiple lines seen
  } 

  //debug
  // RAW_VALUES --> prints each indiv sensor value
  // PROCESSED_AND_CONSECUTIVES --> prints pos value and consectuive lighted
  if (debug == RAW_VALUES) {
    for (int i = 0; i < NUM_SENSORS; i++) {
      Serial.print(sensorReadings[i]);
      Serial.print(", ");
    }
    Serial.println();
  }
  else if (debug == PROCESSED_AND_CONSECUTIVES) {
    Serial.print("position: " + String(pos));
    Serial.println(", consecutive: " + String(highestConsec));
  }
}

void left_edge_trig() {
  latched = LEFT;
}
void right_edge_trig() {
  latched = RIGHT;
}
void check_interrupt() {
  if (latched != NONE) {
    Serial.println(latched);
    drivetrain_setSpeed(0,0);
    latched = NONE;
  }
}



//codes
/*
 *  
  edge_protection = true;
  config_settings(30, 50, 0, 2, 70, PASS, PASS);
  Degrees(1850);
  
  config_settings(30,15,PASS,PASS,50,PASS,PASS);
  Junction();
  
  drivetrain_setSpeed(-60, 60);
  while (digitalReadFast(sensorPins[2]) == HIGH) {}
  while (digitalReadFast(sensorPins[2]) == LOW) {}
  drivetrain_setSpeed(0, 0);
  Serial.println("one ");
  
  //edge_protection = true; 
  config_settings(21, 50, 0, 2, 50, PASS, PASS);
  Degrees(1100);
  Serial.println("two ");
  
  config_settings(25, 50, 0, 2, 70, PASS, PASS);
  Degrees(500);
  Serial.println("three ");
  
  config_settings(25,50,PASS,PASS,60,PASS,PASS);
  Junction();
  
  drivetrain_setSpeed(60, -60);
  while (digitalReadFast(sensorPins[2]) == HIGH) {}
  while (digitalReadFast(sensorPins[2]) == LOW) {}
  drivetrain_setSpeed(0, 0);
  Serial.println("four ");
  
 * 
 * 
 * 
 * 
 */
