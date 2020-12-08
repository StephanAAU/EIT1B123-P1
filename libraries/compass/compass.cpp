
#include "compass.h"
#include <Wire.h>
#include <Adafruit_HMC5883_U.h>

Adafruit_HMC5883_Unified compass = Adafruit_HMC5883_Unified();

bool compassSetup() {
  while (!compass.begin())
  {
    Serial.println("HMC5883 not found");
    delay(1000);
  }

  disableMPU6050();
}

void printCompassSensorDetails()
{
  sensor_t sensor;
  compass.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" uT");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" uT");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" uT");
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}

float getCompassHeading() {
  sensors_event_t event;
  compass.getEvent(&event);

  float heading = atan2(event.magnetic.y, -event.magnetic.z);

  float headingDeg = heading * 180 / M_PI;
  if (headingDeg < 0) {
    headingDeg += 360; 
  }

  return headingDeg;
}

void disableMPU6050() {
  // Disable master mode
  Wire.beginTransmission(0x68);
  Wire.write(0x6A);
  Wire.write(0x00);
  Wire.endTransmission();
  
  // Disable sleep
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();
  
  // enable I2C_BYPASS_EN
  Wire.beginTransmission(0x68);
  Wire.write(0x37);
  Wire.write(0x02);
  Wire.endTransmission();
}
