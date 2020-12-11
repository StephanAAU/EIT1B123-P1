#include <Arduino.h>
#include "pinout.h"

void initUltra() {
	// Defining outputs and inputs.
	pinMode(TRIGPIN, OUTPUT);
	pinMode(ECHOPIN, INPUT);
	digitalWrite(TRIGPIN, LOW);
}

// Returns distance measurement from ultrasonic sensor in cm.
float ultraGetDist() {
	float duration;
	float distance;
	// Clear the trigPin by setting it LOW:
	digitalWrite(TRIGPIN, LOW);
	delayMicroseconds(5);
	// Trigger the sensor by setting the trigPin high for 10 microseconds:
	digitalWrite(TRIGPIN, HIGH);
	delayMicroseconds(10);
	digitalWrite(TRIGPIN, LOW);
	// Read the echoPin, pulseIn() returns the duration (length of the pulse) in microseconds:
	duration = pulseIn(ECHOPIN, HIGH);
	// Calculate the distance:
	distance = (duration * 0.034 / 2)*10;

	return distance;
}
