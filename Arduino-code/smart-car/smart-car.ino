#include <SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial bluetooth(10, 11);
Servo scanner;

const int motorLeft1 = 2;
const int motorLeft2 = 3;
const int motorRight1 = 4;
const int motorRight2 = 5;

const int speedA = 6;
const int speedB = 9;

const int trigPin = 7;
const int echoPin = 8;

const int servoPin = 12;

volatile long leftCount = 0;
volatile long rightCount = 0;

char cmd;
int speedValue = 180;
bool autoModeEnabled = false;

long duration;
int distanceValue;

const float wheelCircumference = 21.0;
const int pulsesPerRevolution = 20;

char authorName[] = {83,117,114,101,115,104,32,83,104,101,114,105,107,97,114,0};

void leftEncoderISR()
{
  leftCount++;
}

void rightEncoderISR()
{
  rightCount++;
}

long cmToPulses(float cm)
{
  return (cm / wheelCircumference) * pulsesPerRevolution;
}

void moveForwardDistance(float cm)
{
  leftCount = 0;
  rightCount = 0;
  long target = cmToPulses(cm);
  digitalWrite(motorLeft1, HIGH);
  digitalWrite(motorLeft2, LOW);
  digitalWrite(motorRight1, HIGH);
  digitalWrite(motorRight2, LOW);
  while (leftCount < target && rightCount < target) {}
  stopRobot();
}

void moveBackwardDistance(float cm)
{
  leftCount = 0;
  rightCount = 0;
  long target = cmToPulses(cm);
  digitalWrite(motorLeft1, LOW);
  digitalWrite(motorLeft2, HIGH);
  digitalWrite(motorRight1, LOW);
  digitalWrite(motorRight2, HIGH);
  while (leftCount < target && rightCount < target) {}
  stopRobot();
}

void turnLeftDistance(int pulses)
{
  leftCount = 0;
  rightCount = 0;
  digitalWrite(motorLeft1, LOW);
  digitalWrite(motorLeft2, HIGH);
  digitalWrite(motorRight1, HIGH);
  digitalWrite(motorRight2, LOW);
  while (rightCount < pulses) {}
  stopRobot();
}

void turnRightDistance(int pulses)
{
  leftCount = 0;
  rightCount = 0;
  digitalWrite(motorLeft1, HIGH);
  digitalWrite(motorLeft2, LOW);
  digitalWrite(motorRight1, LOW);
  digitalWrite(motorRight2, HIGH);
  while (leftCount < pulses) {}
  stopRobot();
}

void setup()
{
  pinMode(motorLeft1, OUTPUT);
  pinMode(motorLeft2, OUTPUT);
  pinMode(motorRight1, OUTPUT);
  pinMode(motorRight2, OUTPUT);

  pinMode(speedA, OUTPUT);
  pinMode(speedB, OUTPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  attachInterrupt(digitalPinToInterrupt(2), leftEncoderISR, RISING);
  attachInterrupt(digitalPinToInterrupt(3), rightEncoderISR, RISING);

  Serial.begin(9600);
  bluetooth.begin(9600);

  scanner.attach(servoPin);
  scanner.write(90);

  stopRobot();

  Serial.println("Robot is starting...");
  Serial.print("Author: ");
  Serial.println(authorName);
}

void loop()
{
  if (bluetooth.available())
  {
    cmd = bluetooth.read();
    processCommand(cmd);
  }

  analogWrite(speedA, speedValue);
  analogWrite(speedB, speedValue);

  if (autoModeEnabled)
  {
    autoNavigation();
  }
}

void processCommand(char command)
{
  switch (command)
  {
    case 'F': autoModeEnabled = false; moveForwardDistance(20); break;
    case 'B': autoModeEnabled = false; moveBackwardDistance(20); break;
    case 'L': autoModeEnabled = false; turnLeftDistance(15); break;
    case 'R': autoModeEnabled = false; turnRightDistance(15); break;
    case 'S': autoModeEnabled = false; stopRobot(); break;
    case 'A': autoModeEnabled = true; break;
    case '1': autoModeEnabled = false; runSchoolRoute(); break;
    case '2': autoModeEnabled = false; runHomeRoute(); break;
    case '3': autoModeEnabled = false; runOfficeRoute(); break;
    case '4': autoModeEnabled = false; runHospitalRoute(); break;
  }
}

int getDistance()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH, 20000);
  if (duration == 0) return 999;
  return duration * 0.034 / 2;
}

void autoNavigation()
{
  distanceValue = getDistance();
  if (distanceValue > 20)
  {
    moveForwardDistance(10);
    return;
  }
  stopRobot();
  moveBackwardDistance(10);
  scanner.write(150);
  delay(300);
  int leftSide = getDistance();
  scanner.write(30);
  delay(300);
  int rightSide = getDistance();
  scanner.write(90);
  if (leftSide > rightSide)
  {
    turnLeftDistance(15);
  }
  else
  {
    turnRightDistance(15);
  }
}

void runSchoolRoute()
{
  moveForwardDistance(100);
  moveForwardDistance(100);
  moveForwardDistance(100);
}

void runHomeRoute()
{
  moveForwardDistance(100);
  turnLeftDistance(15);
  moveForwardDistance(80);
  turnRightDistance(15);
  moveForwardDistance(100);
}

void runOfficeRoute()
{
  moveForwardDistance(120);
  turnLeftDistance(15);
  moveForwardDistance(80);
  turnRightDistance(15);
  moveForwardDistance(90);
  turnLeftDistance(15);
}

void runHospitalRoute()
{
  moveForwardDistance(100);
  turnLeftDistance(15);
  moveForwardDistance(100);
  turnRightDistance(15);
  moveForwardDistance(100);
}

void stopRobot()
{
  digitalWrite(motorLeft1, LOW);
  digitalWrite(motorLeft2, LOW);
  digitalWrite(motorRight1, LOW);
  digitalWrite(motorRight2, LOW);
  analogWrite(speedA, 0);
  analogWrite(speedB, 0);
}
