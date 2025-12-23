// ---------- Ultrasonic ----------
#define TRIG A5
#define ECHO A4

// ---------- Motor Driver Pins ----------
// SPEED PINS (Must be on Digital PWM pins ~)
#define enA 11
#define enB 3

// DIRECTION PINS
// WARNING: Pin 1 and 2 are usually Serial TX/RX. Disconnect before uploading new code!
#define in1 1  
#define in2 2

#define in3 12
#define in4 13

// Define the obstacle threshold distance in cm
const int OBSTACLE_THRESHOLD = 15; 
// Define the speed for turning
const int TURN_SPEED = 180;
// Define the speed for forward movement
const int FORWARD_SPEED = 200;

void setup() {

  // Ultrasonic pins
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  // Motor pins
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT); 
  pinMode(in2, OUTPUT);

  pinMode(enB, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  // Set the default speed for both motors
  analogWrite(enA, FORWARD_SPEED);
  analogWrite(enB, FORWARD_SPEED);
}

// ------------ Motor Functions ------------

// Turns the robot right (Motor A backward, Motor B forward)
void right() {
  analogWrite(enA, TURN_SPEED);
  analogWrite(enB, TURN_SPEED);

  // Motor A (Right Wheel): Backward 
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);

  // Motor B (Left Wheel): Forward 
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}

// Turns the robot left (Motor A forward, Motor B backward)
void left() {
  analogWrite(enA, TURN_SPEED);
  analogWrite(enB, TURN_SPEED);

  // Motor A (Right Wheel): Forward 
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);

  // Motor B (Left Wheel): Backward 
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

// Moves both motors forward
void forward() {
  analogWrite(enA, FORWARD_SPEED);
  analogWrite(enB, FORWARD_SPEED);

  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);

  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}

void backward() {
  analogWrite(enA, FORWARD_SPEED);
  analogWrite(enB, FORWARD_SPEED);

  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);

  digitalWrite(in4, LOW);
  digitalWrite(in3, HIGH);
}

// Stops both motors
void stopCar() {
  analogWrite(enA, 0);
  analogWrite(enB, 0);

  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);

  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

// ------------ Get Distance ------------
long getDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 30000); 
  
  if (duration == 0) return 999; 

  return duration * 0.034 / 2;
}

// ------------ MAIN LOOP ------------
void loop() {
  long distance = getDistance();

  // If obstacle is detected
  if (distance <= OBSTACLE_THRESHOLD) {

    // 1. Stop the car
    stopCar();
    delay(200); 
    backward();
    delay(200);
    // 2. Start turning right (can change to left if preferred)
    right();
    
    // 3. Keep turning until an obstacle-free path is scanned
    long turn_distance = 0;
    int scan_count = 0;
    
    do {
      delay(250); // Small turn duration
      stopCar();
      delay(100); // Stop briefly to get a clean reading
      
      turn_distance = getDistance();

      
      // If obstacle is still present, keep turning
      if (turn_distance <= OBSTACLE_THRESHOLD) {
        right();
        scan_count++;
      }

      // If stuck (turned too many times), break and stop
      if (scan_count > 20) {
        stopCar(); 
        break; 
      }

    } while (turn_distance <= OBSTACLE_THRESHOLD);
    
  } 
  else {
    // No obstacle, continue moving forward
    forward();
  }
}