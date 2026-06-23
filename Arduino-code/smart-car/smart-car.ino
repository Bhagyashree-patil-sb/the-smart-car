#include <BluetoothSerial.h>
#include <ESP32Servo.h>

BluetoothSerial SerialBT;
Servo scanner;

#define ENA 14
#define IN1 27
#define IN2 26

#define ENB 12
#define IN3 25
#define IN4 33

#define TRIG 5
#define ECHO 18

#define SERVO_PIN 13

#define CH_A 0
#define CH_B 1

char author[] = {83,117,114,101,115,104,32,83,104,101,114,105,107,97,114,0};

int motorSpeed = 180;

bool autoMode = false;
bool routeMode = false;
bool routeRunning = false;
int routeID = 0;

unsigned long lastCmdTime = 0;

void setSpeed(int s) {
  motorSpeed = s;
  ledcWrite(CH_A, motorSpeed);
  ledcWrite(CH_B, motorSpeed);
}

void stopCar() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void forward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void backward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void left() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void right() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

long getDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long duration = pulseIn(ECHO, HIGH, 30000);
  long dist = duration * 0.034 / 2;
  return dist;
}

void moveForward(int ms) {
  forward();
  delay(ms);
  stopCar();
}

void turnLeft(int ms) {
  left();
  delay(ms);
  stopCar();
}

void turnRight(int ms) {
  right();
  delay(ms);
  stopCar();
}

void autoDrive() {
  long front;
  scanner.write(90);
  delay(200);
  front = getDistance();

  if (front > 25 || front == 0) {
    forward();
    return;
  }

  stopCar();
  delay(200);

  scanner.write(150);
  delay(300);
  long leftD = getDistance();

  scanner.write(30);
  delay(300);
  long rightD = getDistance();

  scanner.write(90);

  if (leftD > rightD) {
    left();
    delay(400);
  } else {
    right();
    delay(400);
  }

  forward();
}

void routeHome() {
  moveForward(5000);
  turnRight(700);
  moveForward(3000);
  stopCar();
}

void routeSchool() {
  moveForward(3000);
  turnLeft(600);
  moveForward(2000);
  stopCar();
}

void routeOffice() {
  turnRight(800);
  moveForward(2500);
  turnLeft(500);
  moveForward(2000);
  stopCar();
}

void routeHospital() {
  moveForward(2000);
  turnLeft(700);
  moveForward(4000);
  turnRight(600);
  moveForward(1000);
  stopCar();
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_CAR");

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  scanner.attach(SERVO_PIN);
  scanner.write(90);

  ledcSetup(CH_A, 5000, 8);
  ledcAttachPin(ENA, CH_A);

  ledcSetup(CH_B, 5000, 8);
  ledcAttachPin(ENB, CH_B);

  setSpeed(motorSpeed);
}

void loop() {
  if (SerialBT.available()) {
    char cmd = SerialBT.read();
    lastCmdTime = millis();

    if (cmd == 'M') {
      autoMode = false;
      routeMode = false;
      routeRunning = false;
      stopCar();
    }

    if (cmd == 'A') {
      autoMode = true;
      routeMode = false;
      routeRunning = false;
    }

    if (cmd == '1') { routeMode = true; autoMode = false; routeID = 1; routeRunning = false; }
    if (cmd == '2') { routeMode = true; autoMode = false; routeID = 2; routeRunning = false; }
    if (cmd == '3') { routeMode = true; autoMode = false; routeID = 3; routeRunning = false; }
    if (cmd == '4') { routeMode = true; autoMode = false; routeID = 4; routeRunning = false; }

    if (!autoMode && !routeMode) {
      if (cmd == 'F') forward();
      if (cmd == 'B') backward();
      if (cmd == 'L') left();
      if (cmd == 'R') right();
      if (cmd == 'S') stopCar();
    }
  }

  if (millis() - lastCmdTime > 2000) {
    stopCar();
  }

  if (autoMode) {
    autoDrive();
  }

  if (routeMode && !routeRunning) {
    routeRunning = true;

    if (routeID == 1) routeHome();
    if (routeID == 2) routeSchool();
    if (routeID == 3) routeOffice();
    if (routeID == 4) routeHospital();

    routeRunning = false;
    routeMode = false;
  }
}
