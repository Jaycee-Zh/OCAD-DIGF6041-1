/*
A Conversation
The project uses microphones' volumes to control servos' angles and draw lines on a roll of paper.
The movements of servos and lights change according to the two players' actions, producing different images as the feedback.

inputs:
- microphone's volume -> servo's angle
- motion sensor -> whether there is a player

outputs:
- LEDs: to indicate the status
- servos: to move pens to draw
- motor/servo: to move the paper

status(decided by the logic gates on circuit):
- canEcho: 
  if there is no motion, the device will be on canEcho status and react to the other player's input (if there is one)
- isDrawing: 
  if there is motion being detected, and the volume is greater than the min threshold(set by a potentiometer), then the device will be on the isDrawing status. 
  The green LED is on and the servo will move based on player's volume.
- isBlocked: 
  If the other player's input is greater than the max threshold(set by another potentiometer), then the device will be on the isBlocked status. 
  The red LED on the other player's side is on and the servo of this side will stop moving for a given time.

actions(decided by Arduino):
- When either of the players is on the isDrawing status, the motor will roll the paper  
- When both isDrawing is true, servos will move by a large range of angles and the drawn lines will cross with each other.

*/

#include <Servo.h>

// ------ Hardware ------
// ------ inputs
int mic_A_pin = A0;     // pin for microphone
int vol_B_in_pin = A1;  // pin for microphone
// ------ statuses
// for player A
int canEcho_A_pin = 13;
int isDrawing_A_pin = 12;
int isBlocked_A_pin = 11;
// for player B
int canEcho_B_pin = 10;
int isDrawing_B_pin = 9;
int isBlocked_B_pin = 8;
// ------ outputs
// data
int vol_A_out_pin = A2;  // for sending out the vol
// movements
int servo_A_pin = A4;       // for drawing
int paperRolling_pin = A5;  // for paper moving
Servo servo_A;              // create Servo object to control a servo
Servo servo_P;              // create Servo object to control a servo

// ------ settings ------
// ------ about movements
// set the range of servo movement
// !adjust teh values below according to the servos' positions
int pos_min = 130;
int pos_mid = 90;  // slightly
int pos_max = 10;
// set initial position
// !adjust the value below according to the servos' condition, making the drawing arm perpendicular to the paper rolling direction or players' sides.
int pos_initial = 100;
// how much it waves to draw player's attention
int pos_initial_reaction = 4;
// set the paper rolling speed
int speed_initial = 91;  // when a player is detected
int speed = 95;          // when one of the players are drawing
// ------ about time
int interval = 50;         // how long between each draw
int freezingTimer = 2000;  // how long the device will stay on one status
int calTime = 5000;        // how long for calibration
// ------ about experience
const int readSampleNum = 10;  // how many readings for smoothening the inputs; the bigger, the smoother

// ------ variables and their initial values ------
// ------ statuses
bool isDrawing_A = false;
bool isDrawing_B = false;
bool canEcho_A = false;
bool canEcho_B = false;
bool isBlocked_A = false;
bool isBlocked_B = false;

// ------ inputs
int vol_A, vol_A_smoothened, vol_A_out, vol_B, vol_B_out;
// initial min and max value for calibration
int vol_min = 1024;
int vol_max = 0;


// ------ outputs
int pos;

// ------ for calibration and smoothing
int vols[readSampleNum] = { 0 };
int volIndex = 0;
int volCount = 0;
int volTotal;
unsigned long startTime, currentTime, lastTime;

void setup() {

  // get time
  const startTime = millis();
  currentTime = millis();
  lastTime = millis();

  // inputs
  pinMode(mic_A_pin, INPUT);     // get the volume of microphone A
  pinMode(vol_B_in_pin, INPUT);  // get the smoothened volume of microphone B
  pinMode(canEcho_A_pin, INPUT);
  pinMode(isDrawing_A_pin, INPUT);
  pinMode(isBlocked_A_pin, INPUT);
  pinMode(canEcho_B_pin, INPUT);
  pinMode(isDrawing_B_pin, INPUT);
  pinMode(isBlocked_B_pin, INPUT);

  // outputs
  pinMode(vol_A_out_pin, OUTPUT);
  servo_A.attach(servo_A_pin);       // attaches the servo on the pin for drawing
  servo_P.attach(paperRolling_pin);  // attaches the servo on the pin for paper rolling

  // initial position and speed
  servo_A.write(pos_initial);
  servo_P.write(90);  // speed = 0

  // for debugging
  Serial.begin(9600);
}



void loop() {

  // read status
  canEcho_A = digitalRead(canEcho_A_pin);
  isDrawing_A = digitalRead(isDrawing_A_pin);
  isBlocked_A = digitalRead(isBlocked_A_pin);
  canEcho_B = digitalRead(canEcho_B_pin);
  isDrawing_B = digitalRead(isDrawing_B_pin);
  isBlocked_B = digitalRead(isBlocked_B_pin);

  // read input
  vol_B_out = analogRead(vol_B_in_pin);

  servo_P.write(90); 

  // debug
  // vol_A = analogRead(mic_A_pin);
  // Serial.println(vol_A);

  currentTime = millis();

  if (currentTime - startTime < calTime) {  // during the calibrating time
    // calibration input
    calibrateMic();
    // indicate that it is calibrating
    servo_A.write(pos_min);
    // no rolling
    servo_P.write(90);
  } else {  // when calibrating time is over

    // for debugging
    // Serial.println("calibration is done: ")
    // Serial.print(" min is ");
    // Serial.println(vol_min);
    // Serial.print("; max is ");
    // Serial.println(vol_max);

    if (!isDrawing_A && !isDrawing_B) {  // if none of the devices is drawing
      servo_P.write(90);                 // stop rolling paper
    } else {                             // if any of the devices is drawing
      servo_P.write(speed);              // rolling the paper
    };

    if (canEcho_A) {      // on canEcho mode
      if (isDrawing_B) {  // if another device is drawing
        // get smoothened numbers from another Arduino and draw
        draw(vol_B_out, pos_min, pos_mid);
      } else {
        servo_A.write(pos_initial);  // stay
      };
    } else {  // motion is detected
      // indicator: move servo a little bit to tell user that they can interact with the device
      servo_A.write(pos_initial + pos_initial_reaction);
      if (isDrawing_A == true) {  // mic's input is greater than the threshold (set on the circuit)
        // smoothen inputs
        smoothMic();
        analogWrite(vol_A_out_pin, vol_A_smoothened);  // send out the smoothened values
        if (isBlocked_A == true) {
          draw(vol_A_out, pos_min, pos_max);
          delay(freezingTimer);  // stop reacting for a given time
        } else {
          vol_A_out = vol_A_smoothened;
          if (isDrawing_B == true) {  // player B also has input, full range
            draw(vol_A_out, pos_min, pos_max);
          } else {  // only player A is drawing, small range
            draw(vol_A_out, pos_min, pos_mid);
          }
        }
      }
    }
  }
}

// calibrate the min and max volume for the microphone
void calibrateMic() {
  int vol = analogRead(mic_A_pin);
  // Read the value and define min and max
  if (vol < vol_min) {
    vol_min = vol;
  };
  if (vol > vol_max) {
    vol_max = vol;
  };

  // for debugging
  // Serial.print("; min is ");
  // Serial.println(vol_min);
  // Serial.print("; max is ");
  // Serial.println(vol_max);
}

// smoothen the volume values
void smoothMic() {
  for (int i = 0; i < readSampleNum; i++) {
    int vol = analogRead(mic_A_pin);
    volTotal -= vols[volIndex];
    vols[volIndex] = vol;
    volTotal += vol;
    volIndex = (volIndex + 1) % readSampleNum;
    vol_A_smoothened = volTotal / readSampleNum;
    // delay(interval);
    // return vol_A_smoothened;
  };
  // debug
  //  Serial.println(vols);
}

void draw(int value, int min, int max) {
  pos = constrain(map(value, vol_min, vol_max, min, max), min, max);  // map values to positions
  servo_A.write(pos);                                                 // drawing
  delay(interval);                                                    // pause a little to smoothen the movement
}