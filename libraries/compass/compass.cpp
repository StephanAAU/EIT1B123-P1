
#include "compass.h"
#include <Wire.h>
#include <Adafruit_HMC5883_U.h>

Adafruit_HMC5883_Unified compass = Adafruit_HMC5883_Unified();

bool MASTER;

bool compassSetup(bool master) {
  MASTER = master;

  while (!compass.begin())
  {
    Serial.println("HMC5883 not found");
    delay(1000);
  }

  disableMPU6050();
  return 1;
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

Vector getCorrectedCompassReadings(sensors_event_t *event, struct DistortionValues *dv) {
  Vector v;
  v.x = ((*event).magnetic.x - (*dv).oX) * (*dv).sX;
  v.y = ((*event).magnetic.y - (*dv).oY) * (*dv).sY;
  v.z = ((*event).magnetic.z - (*dv).oZ) * (*dv).sZ;
  return v;
}

float getCompassHeading(struct DistortionValues *dv) {
  sensors_event_t event = getCompassEvent();

  float x = event.magnetic.x;
  float y = event.magnetic.y;
  float z = event.magnetic.z;
  if (dv && ((*dv).set)) {
    Vector v = getCorrectedCompassReadings(&event, dv);
    x = v.x;
    y = v.y;
    z = v.z;
  }

  float heading;
  if (MASTER) {
    heading = atan2(x, -y);
  } else {
    heading = atan2(y, -z);
  }

  float headingDeg = heading * 180 / M_PI;
  if (headingDeg < 0) {
    headingDeg += 360;
  }

  return headingDeg;
}

sensors_event_t getCompassEvent() {
  sensors_event_t event;
  compass.getEvent(&event);

  return event;
}

void updateMinMaxVector(struct MinMaxVector *mmv, sensors_event_t *event) {
  float x = (*event).magnetic.x;
  float y = (*event).magnetic.y;
  float z = (*event).magnetic.z;

  if (x < (*mmv).min.x) {
    (*mmv).min.x = x;
  }
  if (y < (*mmv).min.y) {
    (*mmv).min.y = y;
  }
  if (z < (*mmv).min.z) {
    (*mmv).min.z = z;
  }
  
  if (x > (*mmv).max.x) {
    (*mmv).max.x = x;
  }
  if (y > (*mmv).max.y) {
    (*mmv).max.y = y;
  }
  if (z > (*mmv).max.z) {
    (*mmv).max.z = z;
  }
}

struct DistortionValues getDistortionValues(struct MinMaxVector *mmv) {
  struct DistortionValues dv;

  Serial.println("MinMax values:");
  Serial.printf("minX=%.2f minY=%.2f minZ=%.2f maxX=%.2f maxY=%.2f maxZ=%.2f", (*mmv).min.x, (*mmv).min.y, (*mmv).min.z, (*mmv).max.x, (*mmv).max.y, (*mmv).max.z);
  Serial.println("");
  
  // offsets
  dv.oX = ((*mmv).max.x + (*mmv).min.x) / 2;
  dv.oY = ((*mmv).max.y + (*mmv).min.y) / 2;
  dv.oZ = ((*mmv).max.z + (*mmv).min.z) / 2;

  float avgDeltaX = ((*mmv).max.x - (*mmv).min.x) / 2;
  float avgDeltaY = ((*mmv).max.y - (*mmv).min.y) / 2;
  float avgDeltaZ = ((*mmv).max.z - (*mmv).min.z) / 2;
  float avgDelta = (avgDeltaX + avgDeltaY + avgDeltaZ) / 3;

  // scales
  dv.sX = avgDelta / avgDeltaX;
  dv.sY = avgDelta / avgDeltaY;
  dv.sZ = avgDelta / avgDeltaZ;

  Serial.println("Distortion values:");
  Serial.printf("oX=%.2f oY=%.2f oZ=%.2f - sX=%.2f sY=%.2f sZ=%.2f", dv.oX, dv.oY, dv.oZ, dv.sX, dv.sY, dv.sZ);
  Serial.println("");

  return dv;
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
