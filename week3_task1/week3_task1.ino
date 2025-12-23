#include <LiquidCrystal.h>

// --- PINS ---
#define enA 11
#define in1 13
#define in2 12
#define enB 3
#define in3 2
#define in4 1

const int encoderRightPin = A1;
const int encoderLeftPin = A2;
const int midIrPin = A3;
const int leftIrPin = A4;
const int rightIrPin = A5;

#define LEFT_THRES 550
#define RIGHT_THRES 550
#define MID_THRES 500

#define FORWARD 1
#define BACKWARD 0
#define CM_PER_COUNT 0.5

#define STATE_RUNNING_TO_20 0
#define STATE_PAUSED 1
#define STATE_RUNNING_TO_END 2
#define STATE_FINISHED 3

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

volatile unsigned long rightCount = 0;
volatile unsigned long leftCount = 0;

int currentState = STATE_RUNNING_TO_20;

int last = 0;
int turnSpeed = 150;
int straightSpeed = 100;

unsigned long totalRunTime = 0;
unsigned long lastLoopTime = 0;
unsigned long pauseStartTime = 0;

int stopCounter = 0;
int Counter = 0;

void rotateMotorA(int speed, int direction) {
  speed = constrain(speed, 0, 255);
  analogWrite(enA, speed);
  if (direction == FORWARD) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  }
}

void rotateMotorB(int speed, int direction) {
  speed = constrain(speed, 0, 255);
  analogWrite(enB, speed);
  if (direction == FORWARD) {
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
  } else {
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
  }
}

void stopMotors() {
  analogWrite(enA, 0);
  analogWrite(enB, 0);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

void displayStats(unsigned long sec, unsigned long ms, float dist) {

  char formattedTime[8];
  sprintf(formattedTime, "%02ld:%03ld", sec, ms);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  lcd.setCursor(6, 0);
  lcd.print(formattedTime);

  lcd.setCursor(0, 1);
  lcd.print("Dist: ");
  lcd.print(dist);
  lcd.print(" cm");
}


void lineFollowLogic(int leftVal, int midVal, int rightVal) {
  if (leftVal > LEFT_THRES && rightVal > RIGHT_THRES && midVal < MID_THRES) {
    Counter++;
    lcd.setCursor(8, 1);
    lcd.print("count:");
    lcd.print(Counter);
    if (Counter > 15) {
      if (last == 1) {
        rotateMotorA(200, FORWARD);
        rotateMotorB(200, BACKWARD);
      } else if (last == 2) {
        rotateMotorA(200, BACKWARD);
        rotateMotorB(200, FORWARD);
      }
      Counter = 0;
    }
  } else {
    if (leftVal < LEFT_THRES) {
      last = 1;
      rotateMotorA(0, BACKWARD);
      rotateMotorB(turnSpeed, BACKWARD);
    } else if (rightVal < RIGHT_THRES) {
      last = 2;
      rotateMotorA(turnSpeed, BACKWARD);
      rotateMotorB(0, BACKWARD);
    } else if (midVal > MID_THRES) {
      rotateMotorA(straightSpeed, BACKWARD);
      rotateMotorB(straightSpeed, BACKWARD);
    } else {
      Counter = 0;
    }
  }
}

ISR(PCINT1_vect) {
  uint8_t portState = PINC;
  int rightState = (portState >> 1) & 1;
  int leftState = (portState >> 2) & 1;
  static int lastRightState = 0;
  static int lastLeftState = 0;
  if (rightState != lastRightState) rightCount++;
  if (leftState != lastLeftState) leftCount++;
  lastRightState = rightState;
  lastLeftState = leftState;
}

void setup() {
  lcd.begin(16, 2);
  lcd.clear();

  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(rightIrPin, INPUT);
  pinMode(leftIrPin, INPUT);
  pinMode(midIrPin, INPUT);
  pinMode(encoderRightPin, INPUT);
  pinMode(encoderLeftPin, INPUT);

  PCICR |= (1 << PCIE1);
  PCMSK1 |= (1 << PCINT9) | (1 << PCINT10);

  stopMotors();
  lastLoopTime = millis();
}

void loop() {
  unsigned long currentMillis = millis();

  float avgCounts = (leftCount + rightCount) / 2.0;
  float distance = avgCounts * CM_PER_COUNT;

  int leftVal = analogRead(leftIrPin);
  int midVal = analogRead(midIrPin);
  int rightVal = analogRead(rightIrPin);


  if (currentState != STATE_FINISHED) {
    totalRunTime += (currentMillis - lastLoopTime);
  }
  lastLoopTime = currentMillis;

  unsigned long seconds = totalRunTime / 1000;
  unsigned long remainder = totalRunTime % 1000;

  switch (currentState) {

    case STATE_RUNNING_TO_20:
      lineFollowLogic(leftVal, midVal, rightVal);
      displayStats(seconds, remainder, distance);

      if (distance >= 16.0) {
        stopMotors();
        pauseStartTime = currentMillis;
        currentState = STATE_PAUSED;
        lcd.clear();
      }
      break;

    case STATE_PAUSED:
      stopMotors();
      distance = 20.09;
      unsigned long timeInPause;
      timeInPause = currentMillis - pauseStartTime;

      if (timeInPause >= 3000) {
        rotateMotorA(straightSpeed, BACKWARD);
        rotateMotorB(straightSpeed, BACKWARD);
        delay(50);
        currentState = STATE_RUNNING_TO_END;
        lcd.clear();
      } else {
        int remainingSeconds = 3 - timeInPause / 1000;
        lcd.setCursor(0, 0);
        lcd.print("Wait: ");
        lcd.print(remainingSeconds);
        lcd.print(" s");

        lcd.setCursor(0, 1);
        lcd.print("Dist: ");
        lcd.print(distance);
      }
      break;

    case STATE_RUNNING_TO_END:



      if (leftVal < LEFT_THRES && rightVal < RIGHT_THRES && midVal > MID_THRES) {
        stopCounter++;
        if (stopCounter > 1) {
          stopMotors();
          currentState = STATE_FINISHED;
          while (1)
            ;
          lcd.clear();
        }
      } else {
        stopCounter = 0;
        lineFollowLogic(leftVal, midVal, rightVal);
        displayStats(seconds, remainder, distance);
      }




      break;

    case STATE_FINISHED:
      stopMotors();
      char formattedTime[8];
      sprintf(formattedTime, "%02ld:%03ld", seconds, remainder);

      lcd.setCursor(0, 0);
      lcd.print("Time: ");
      lcd.setCursor(6, 0);
      lcd.print(formattedTime);

      lcd.setCursor(0, 1);
      lcd.print("Dist: ");
      lcd.print(distance);
      lcd.print(" cm");
      break;
  }
}
