#ifndef PINOUT_H_
#define PINOUT_H_

// Range finder shutdown pins.
const int SHDN_RANGE1 = 16;
const int SHDN_RANGE2 = 17;

// Define stepper pin connections & motor's steps per revolution
const int dirPin = 23;
const int stepPin = 19;
const int stepsPerRevolution = 400;

// Defines til kegle afvigelse
const float minKegleAfvigelse = 50; // Afvigelse i mm

// Defines til lift ændring
const float minLiftForskel = 80; // Afvigelse i mm

#endif
