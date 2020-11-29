#include <Arduino.h>
#include "motor.h"
#include "pinout.h"

int Pwm1Value = 100;
int Pwm2Value = 100;

int turnStart = 0;
int turnEnd = 0;
bool turning = false;

int driveStart = 0;
int driveEnd = 0;
bool driving = false;

void motorsRun() {
  Pwm1Value = 100;
  Pwm2Value = 100;

  digitalWrite(MOTOR_INA1, LOW);
  digitalWrite(MOTOR_INB1, HIGH);

  digitalWrite(MOTOR_INA2, LOW);
  digitalWrite(MOTOR_INB2, HIGH);

  updatePWMValues();
}

void stopMotors() {
  stopMotor(1);
  stopMotor(2);

  Serial.println("STOP");
}

void stopMotor(int motor) {
  if (motor == 1) {
    Pwm1Value = 0;
  } else if (motor == 2) {
    Pwm2Value = 0;
  }
  
  updatePWMValues();
}

void drive(int cm) {
  stopMotors();

  Pwm1Value = 100;
  Pwm2Value = 100;
  driving = true;

  int k = cm * 30; // constant will wary depending on DC-motors, PWM value, voltage, wheel size ect.

  driveStart = millis();
  driveEnd = driveStart + k;

  digitalWrite(MOTOR_INA1, LOW);
  digitalWrite(MOTOR_INB1, HIGH);

  digitalWrite(MOTOR_INA2, LOW);
  digitalWrite(MOTOR_INB2, HIGH);
  
  updatePWMValues();
}

void verifyDrive() {
  if (driving && driveEnd < millis()) {
    driving = false;
    Serial.println("verifyDrive stop");
    stopMotors();
  }
}

void turn(int deg) {
  stopMotors();

  Pwm1Value = 100;
  Pwm2Value = 100;
  turning = true;

  int k = deg * 5; // constant will wary depending on DC-motors, PWM value, voltage, wheel size ect.

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

  updatePWMValues();
}

void verifyTurn() {
  if (turning && turnEnd < millis()) {
    turning = false;
    Serial.println("verifyTurn stop");
    stopMotors();
  }
}

void PwmSetup() {
  const int freq = 1000;
  const int resolution = 8;

  ledcSetup(MOTOR_PWM_1_CHANNEL, freq, resolution);
  ledcSetup(MOTOR_PWM_2_CHANNEL, freq, resolution);

  ledcAttachPin(MOTOR_PWM_1, MOTOR_PWM_1_CHANNEL);
  ledcAttachPin(MOTOR_PWM_2, MOTOR_PWM_2_CHANNEL);
}

void updatePWMValues() {
  ledcWrite(MOTOR_PWM_1_CHANNEL, Pwm1Value);
  ledcWrite(MOTOR_PWM_2_CHANNEL, Pwm2Value);
}
