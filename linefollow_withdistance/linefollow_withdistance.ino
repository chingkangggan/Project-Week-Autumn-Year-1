#include <LiquidCrystal.h>

#define enA 11  // PWM pin for speed control (left)
#define in1 13  // Input pin 1
#define in2 12  // Input pin 2

#define enB 3  // PWM pin for speed control (right)
#define in3 2  // Input pin 3
#define in4 1  // Input pin 4

const int lineFollowRightPin = A5;  
const int lineFollowLeftPin = A4;   

const int encoderRightPin = A1;  
const int encoderLeftPin = A2;  

#define FORWARD 1
#define BACKWARD 0


#define CM_PER_COUNT 0.519  // (pi*66.1/10)/20/2

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

int state = 0; 
unsigned long lastMillis;

volatile unsigned long rightCount = 0;  
volatile unsigned long leftCount = 0;   
int stopCounter = 0;

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

  int rightState = (portState >> 1) & 1; 
  int leftState = (portState >> 2) & 1;

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

  pinMode(lineFollowRightPin, INPUT);  
  pinMode(lineFollowLeftPin, INPUT);   
  pinMode(encoderRightPin, INPUT);    
  pinMode(encoderLeftPin, INPUT);      


  PCICR |= (1 << PCIE1);                    
  PCMSK1 |= (1 << PCINT9) | (1 << PCINT10); 

  stopMotors();  

  lastMillis = millis();
}

void loop() {
  lcd.clear();

  int leftVal = analogRead(lineFollowLeftPin);
  int rightVal = analogRead(lineFollowRightPin); 
  if (leftVal < 430 && rightVal < 430) {

    float avgCounts = (leftCount + rightCount) / 2.0;
    float distance = avgCounts * CM_PER_COUNT;

    unsigned long currentMillis = millis();
    unsigned long milliseconds = (currentMillis - lastMillis) % 1000;
    unsigned long seconds = (currentMillis - lastMillis) / 1000;
    char formattedTime[8];
    sprintf(formattedTime, "%02ld:%03ld", seconds, milliseconds);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Time: ");
    lcd.setCursor(6, 0);
    lcd.print(formattedTime);
    lcd.setCursor(0, 1);
    lcd.print("Dist: ");
    lcd.print(distance);
    lcd.print(" cm");

    stopCounter++;
    if (stopCounter > 5) {
      while (1) {
        stopMotors();
      }
    }
  } else {
    stopCounter = 0;
  }
  // lcd.setCursor(0, 0);
  // lcd.print("L:");
  // lcd.print(leftVal);
  // lcd.setCursor(8, 0);
  // lcd.print("R:");
  // lcd.print(rightVal);

  if (state == 0) {
    rotateMotorA(0, FORWARD);
    rotateMotorB(150, FORWARD); 
  } else {
    rotateMotorA(150, FORWARD);
    rotateMotorB(0, FORWARD);
  }

  if (leftVal < 430) {
    state = 0;  
  } else if (rightVal < 530) {
    state = 1; 
  }


  float avgCounts = (leftCount + rightCount) / 2.0;
  float distance = avgCounts * CM_PER_COUNT;

  lcd.setCursor(0, 1);
  lcd.print("Dist: ");
  lcd.print(distance);
  lcd.print(" cm");
}