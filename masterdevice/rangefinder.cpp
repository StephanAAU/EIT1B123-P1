#include "rangefinder.h"

#include <math.h>
#include <Wire.h>
#include <VL53L1X.h>
#include "pinout.h"

VL53L1X sensor;
VL53L1X sensor2;

void afstandTest() {
  int s1Timeouts = 0;
  int s2Timeouts = 0;
  
  while (1) {
    float s1 = sensor.read();
    float s2 = sensor2.read();

    Serial.print("s1: ");
    if (sensor.timeoutOccurred()) {
      Serial.print("s1 timeout");
      s1Timeouts++;

      if (s1Timeouts > 5) {
        if (sensor.init()) {
          Serial.print("init ok");
          s1Timeouts = 0;
        } else {
          Serial.print("init fail");
        }
      }
    } else {
      Serial.print(s1);
    }
    Serial.print(" - s2:");
    if (sensor2.timeoutOccurred()) {
      Serial.print("s2 timeout");
      s2Timeouts++;

      if (s2Timeouts > 5) {
        if (sensor2.init()) {
          Serial.print("init ok");
          s2Timeouts = 0;
        } else {
          Serial.print("init fail");
        }
      }
    } else {
      Serial.print(s2);
    }
    Serial.println("");
    
    delay(500);
  }
}

void findAfstand(int x, float *minValue, float *stepLock) {
  float y;
  y = sensor2.read();
  if (y < *minValue) {
    *minValue = y;
    *stepLock = x;
    Serial.println("minValue er ");
    Serial.println(*minValue);
  }
}

float beregnAfstandTilKegle(float x) {
    return (tan(0.5235988)*x);
}

float afstandTilLift() {
  float y;
  y = sensor.read();
  if (sensor.timeoutOccurred()) { 
    Serial.print("sensor1 TIMEOUT"); 
  }
  return y;
}

float afstandTilKegle() {
  float y;
  y = sensor2.read();
  if (sensor2.timeoutOccurred()) { 
    Serial.println("sensor2 TIMEOUT"); 
  }
  return y;
}

void initRangefinders() {
	// Initializing rangefinger 1.
	// Preparation of pins.
	pinMode(SHDN_RANGE1,OUTPUT);
	pinMode(SHDN_RANGE2,OUTPUT);
  
  digitalWrite(SHDN_RANGE1,LOW);
  digitalWrite(SHDN_RANGE2,LOW);
  delay(100);
	digitalWrite(SHDN_RANGE1,HIGH);
  Wire.begin();
  Wire.setClock(400000); // use 400 kHz I2C
	delay(500);
	sensor.setTimeout(500);
	while (!sensor.init())	{
	  Serial.println("Failed to detect and initialize sensor 1!");
    delay(500);
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
	digitalWrite(SHDN_RANGE2,HIGH);
	delay(500);
	sensor2.setTimeout(500);
	while (!sensor2.init())	{
	  Serial.println("Failed to detect and initialize sensor 2!");
    delay(500);
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
