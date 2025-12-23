#include <LiquidCrystal.h>

#define enA 11  // PWM pin for speed control (left)
#define in1 13  // Input pin 1
#define in2 12  // Input pin 2

#define enB 3  // PWM pin for speed control (right)
#define in3 2  // Input pin 3
#define in4 1  // Input pin 4

const int encoderRightPin = A5;  
const int encoderLeftPin = A4;  

#define FORWARD 1
#define BACKWARD 0

#define CM_PER_COUNT 0.519  // (pi*66.1/10)/20/2

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

volatile unsigned long rightCount = 0;  
volatile unsigned long leftCount = 0;   

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
  speed = constrain(speed, 0, 255);  // [cite: 46]
  analogWrite(enB, speed);
  if (direction == FORWARD) {
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
  } else if (direction == BACKWARD) {
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);

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

ISR(PCINT1_vect) {
  uint8_t portState = PINC; 

  int rightState = (portState >> 5) & 1; 
  int leftState = (portState >> 4) & 1;

  static int lastRightState = 0;
  static int lastLeftState = 0;

  if (rightState != lastRightState) {
    rightCount++;  
  }
  if (leftState != lastLeftState) {
    leftCount++;  
  }

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

  pinMode(encoderRightPin, INPUT);    
  pinMode(encoderLeftPin, INPUT);      


  PCICR |= (1 << PCIE1);                    
  PCMSK1 |= (1 << PCINT12) | (1 << PCINT13); 

  stopMotors();  

  delay(1000);

  rotateMotorA(255, 1);
  rotateMotorB(255, 1);
  delay(3000);
  stopMotors();
  float avgCounts = (leftCount + rightCount) / 2.0;
  float distance = avgCounts * CM_PER_COUNT;

  lcd.setCursor(0, 1);
  lcd.print("Dist: ");
  lcd.print(distance);
  lcd.print(" cm");
}

void loop() {
}
