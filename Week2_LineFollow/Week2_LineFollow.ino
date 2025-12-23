#include <LiquidCrystal.h>

#define enA 11  // PWM pin for speed control (left)
#define in1 13  // Input pin 1
#define in2 12  // Input pin 2

#define enB 3  // PWM pin for speed control (right)
#define in3 2  // Input pin 3
#define in4 1  // Input pin 4

const int rightIrPin = A5;  // Right IR sensor on A1 (PC1)
const int leftIrPin = A4;   // Left IR sensor on A2 (PC2)

#define FORWARD 1
#define BACKWARD 0

#define LEFTBLACKTHRES 300
#define LEFTWHITETHRES 700
#define RIGHTBLACKTHRES 500
#define RIGHTWHITETHRES 700

// Initialize the LCD (RS, enable, d4, d5, d6, d7)
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

unsigned long lastMillis;
int state = 0;

void rotateMotorA(int speed, int direction) {
  speed = constrain(speed, 0, 255);  
  analogWrite(enA, speed);
  if (direction == FORWARD) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  } else if (direction == BACKWARD) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  }
}

void rotateMotorB(int speed, int direction) {
  speed = constrain(speed, 0, 255);  
  analogWrite(enB, speed);
  if (direction == FORWARD) {
    digitalWrite(in4, HIGH);
  } else if (direction == BACKWARD) {
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    digitalWrite(in3, LOW);
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
  stopMotors();
}

void loop() {
  lcd.clear();
  int leftVal = analogRead(leftIrPin);
  int rightVal = analogRead(rightIrPin);
  lcd.setCursor(0, 0);
  lcd.print(leftVal);
  lcd.setCursor(0, 1);
  lcd.print(rightVal);
  lcd.setCursor(8, 0);

  if (state == 0) {
    rotateMotorA(0, FORWARD);
    rotateMotorB(150, FORWARD);
  } else {
    rotateMotorA(150, BACKWARD);
    rotateMotorB(0, FORWARD);
  }

  if (leftVal < 430) {
    state = 0;
  } else if (rightVal < 500) {
    state = 1;
  }
}
