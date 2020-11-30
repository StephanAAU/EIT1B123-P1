#ifndef HW_RANGEFINDER_H
#define HW_RANGEFINDER_H

#include "rangefinder.h"

#include <math.h>
#include <Wire.h>
#include <VL53L1X.h>

VL53L1X sensor;
VL53L1x sensor2;

float afstandTilKegle(float x) {
    return (tan(0.5235988)*x);
}

float afstandTilLift() {
  float y;
  y = sensor.read();
  if (sensor.timeoutOccurred()) { 
    Serial.print(" TIMEOUT"); 
  }
  return y;
}
  
void initRangefinders() {
	// Initializing rangefinger 1.
	// Preparation of pins.
	pinMode(SHDN_RANGE1,OUTPUT);
	pinMode(SHDN_RANGE2,OUTPUT);
	pinMode(SHDN_RANGE1,LOW);
	pinMode(SHDN_RANGE2,LOW);
	delay(10);
	pinMode(SHDN_RANGE1,HIGH);
	delay(50);
	sensor.setTimeout(500);
	if (!sensor.init())	{
	Serial.println("Failed to detect and initialize sensor!");
		while (1);
	}
	// Change sensor address to allow multi connection.
	sensor.setAddress(0x40);

	// Use long distance mode and allow up to 50000 us (50 ms) for a measurement.
	// You can change these settings to adjust the performance of the sensor, but
	// the minimum timing budget is 20 ms for short distance mode and 33 ms for
	// medium and long distance modes. See the VL53L1X datasheet for more
	// information on range and timing limits.
	sensor.setDistanceMode(VL53L1X::Long);
	sensor.setMeasurementTimingBudget(50000);

	// Start continuous readings at a rate of one measurement every 50 ms (the
	// inter-measurement period). This period should be at least as long as the
	// timing budget.
	sensor.startContinuous(50);

	// Preparation of pins.
	delay(10);
	pinMode(SHDN_RANGE2,HIGH);
	delay(50);
	sensor2.setTimeout(500);
	if (!sensor2.init())	{
	Serial.println("Failed to detect and initialize sensor!");
		while (1);
	}
	// Change sensor address to allow multi connection.
	sensor2.setAddress(0x41);

	// Use long distance mode and allow up to 50000 us (50 ms) for a measurement.
	// You can change these settings to adjust the performance of the sensor, but
	// the minimum timing budget is 20 ms for short distance mode and 33 ms for
	// medium and long distance modes. See the VL53L1X datasheet for more
	// information on range and timing limits.
	sensor2.setDistanceMode(VL53L1X::Long);
	sensor2.setMeasurementTimingBudget(50000);

	// Start continuous readings at a rate of one measurement every 50 ms (the
	// inter-measurement period). This period should be at least as long as the
	// timing budget.
	sensor2.startContinuous(20);
}

#endif
