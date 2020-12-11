#include <math.h>
#include <Arduino.h>
#include "pinout.h"
#include "stepper.h"
#include "rangefinder.h"

float minValue = 4000, stepLock = 0, stepLockGrader = 0, laengdeAfvigelse = 0, A = 0, B = 0, drejeKegleVinkel = 0;

void stepperSetup() {
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
}
  
void medUret() {
  // Set motor direction clockwise
  digitalWrite(dirPin, LOW);

  // Spin motor speed
  for(int x = 0; x < stepsPerRevolution*0.75; x++)
  {
    findAfstand(x, &minValue, &stepLock);
    digitalWrite(stepPin, HIGH);
    delay(1);
    digitalWrite(stepPin, LOW);
    delay(1);
  }
}

void modUret() {
  // Set motor direction counterclockwise
  digitalWrite(dirPin, HIGH);

  // Spin motor speed
  for(int x = stepsPerRevolution*0.75; x > 0; x--)
  {
    findAfstand(x, &minValue, &stepLock);
    digitalWrite(stepPin, HIGH);
    delay(1);
    digitalWrite(stepPin, LOW);
    delay(1);
  }
}

float findAfvigendeLaengde(float akselsVaerdi) {
  float favPosition = 150 * 0.9;
  stepLockGrader = stepLock * 0.9;
  float stepLockRadian = 0;
  float x = 0, y = 0, x2 = 0, y2 = 0;
  if (stepLockGrader > favPosition + 5*0.9 or stepLockGrader < favPosition - 5*0.9) {
    stepLockRadian = (stepLockGrader * 3.14159) / 180;
    favPosition = (favPosition * 3.14159) / 180;
    x = cos(stepLockRadian) * minValue;
    x2 = cos(favPosition) * akselsVaerdi;
    A = x2 - x;
    y = sin(stepLockRadian) * minValue;
    y2 = sin(favPosition) * akselsVaerdi;
    B = y2 - y;
    laengdeAfvigelse = sqrt(A*A + B*B);
    Serial.printf("laengdeAfvigelse: %.2f \t", laengdeAfvigelse);
    return laengdeAfvigelse;
  }
  else {
    return 0;
  }
}

float findDrejeVinkel () {
  Serial.printf("stepLockGrader: %.2f \t", stepLockGrader);
  drejeKegleVinkel = atan(B/A);
  drejeKegleVinkel = drejeKegleVinkel * 100;
  Serial.printf("drejeKegleVinkel: %.2f \n", drejeKegleVinkel);
  return drejeKegleVinkel;
}

void resetVinkel() {
  minValue = 4000;
  stepLock = 0;
  A = 0;
  B = 0;
}
