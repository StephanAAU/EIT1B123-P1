#include <Adafruit_HMC5883_U.h>

#ifndef COMPASS_H_
#define COMPASS_H_

extern Adafruit_HMC5883_Unified compass;

bool compassSetup(bool master);
void printCompassSensorDetails();
float getCompassHeading();
void disableMPU6050();

#endif // COMPASS_H_
