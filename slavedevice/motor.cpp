#include <Arduino.h>
#include "motor.h"
#include "pinout.h"
#include "ultra.h"

int pwmValue1;
int pwmValue2;

int turnStart = 0;
int turnEnd = 0;
bool turning = false;

int driveStart = 0;
int driveEnd = 0;
bool driving = false;

void motorsRun() {

  digitalWrite(MOTOR_INA1, LOW);
  digitalWrite(MOTOR_INB1, HIGH);

  digitalWrite(MOTOR_INA2, LOW);
  digitalWrite(MOTOR_INB2, HIGH);

  updateMotorDutyCycle(40, 40);
}

void stopMotors() {
  stopMotor(1);
  stopMotor(2);

  Serial.println("STOP");
}

void stopMotor(int motor) {
  if (motor == 1) {
    updateMotorDutyCycle(0, -1);
  } else if (motor == 2) {
    updateMotorDutyCycle(-1, 0);
  }
}

void drive(int mm) {
  stopMotors();

  driving = true;

  int k = (mm/100) * 400; // constant will wary depending on DC-motors, PWM value, voltage, wheel size ect.

  driveStart = millis();
  driveEnd = driveStart + k;

  digitalWrite(MOTOR_INA1, LOW);
  digitalWrite(MOTOR_INB1, HIGH);

  digitalWrite(MOTOR_INA2, LOW);
  digitalWrite(MOTOR_INB2, HIGH);
  
  updateMotorDutyCycle(40, 40);
}

bool verifyDrive() {
  if (driving && driveEnd < millis()) {
    driving = false;
    Serial.println("verifyDrive stop");
    stopMotors();
    return true;
  }
  if (ultraGetDist() < obstacleDist) {
    driving = false;
    Serial.println("Obstacle stopped (verifyDrive)");
    stopMotors();
    return true;
  }
  return false;
}

void turn(int deg) {
  stopMotors();

  turning = true;

  int k = deg * 10; // constant will wary depending on DC-motors, PWM value, voltage, wheel size ect.

  turnStart = millis();
  turnEnd = turnStart + k;

  if (deg >= 0) {
    digitalWrite(MOTOR_INA1, HIGH);
    digitalWrite(MOTOR_INB1, LOW);
    digitalWrite(MOTOR_INA2, LOW);
    digitalWrite(MOTOR_INB2, HIGH);
  } else {
    digitalWrite(MOTOR_INA1, LOW);
    digitalWrite(MOTOR_INB1, HIGH);
    digitalWrite(MOTOR_INA2, HIGH);
    digitalWrite(MOTOR_INB2, LOW);
  }

  updateMotorDutyCycle(15, 15);
}

bool verifyTurn() {
  if (turning && turnEnd < millis()) {
    turning = false;
    Serial.println("verifyTurn stop");
    stopMotors();
    return true;
  }
  return false;
}

void motorSetup() {
  const int freq = 100;

  ledcSetup(MOTOR_PWM_1_CHANNEL, freq, PWM_RESOLUTION);
  ledcSetup(MOTOR_PWM_2_CHANNEL, freq, PWM_RESOLUTION);

  ledcAttachPin(MOTOR_PWM_1, MOTOR_PWM_1_CHANNEL);
  ledcAttachPin(MOTOR_PWM_2, MOTOR_PWM_2_CHANNEL);
  
  pinMode(MOTOR_INA1, OUTPUT);
  pinMode(MOTOR_INB1, OUTPUT);

  pinMode(MOTOR_INA2, OUTPUT);
  pinMode(MOTOR_INB2, OUTPUT);

  updateMotorDutyCycle(0, 0);
}

void updateMotorDutyCycle(int motor1DutyCycle, int motor2DutyCycle) {
  int pwm1 = motor1DutyCycle * pow(2, PWM_RESOLUTION) / 100;
  int pwm2 = motor2DutyCycle * pow(2, PWM_RESOLUTION) / 100;

  if (motor1DutyCycle == -1) {
    pwm1 = pwmValue1;
  }
  if (motor2DutyCycle == -1) {
    pwm2 = pwmValue2;
  }

  pwmValue1 = pwm1;
  pwmValue2 = pwm2;
  
  ledcWrite(MOTOR_PWM_1_CHANNEL, pwm1);
  ledcWrite(MOTOR_PWM_2_CHANNEL, pwm2);
}
