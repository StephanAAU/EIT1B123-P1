#include <Adafruit_HMC5883_U.h>

#ifndef COMPASS_H_
#define COMPASS_H_

#define FLT_MAX 1000000//3.402823466e+38F /* max value */
#define FLT_MIN -1000000//1.175494351e-38F /* min positive value */

extern Adafruit_HMC5883_Unified compass;

struct Vector {
  float x;
  float y;
  float z;
};

struct MinMaxVector {
  struct Vector min = {
    .x = FLT_MAX,
    .y = FLT_MAX,
    .z = FLT_MAX
  };
  struct Vector max = {
    .x = FLT_MIN,
    .y = FLT_MIN,
    .z = FLT_MIN
  };
};

struct DistortionValues {
  float oX; // offset
  float oY;
  float oZ;
  float sX; // scale
  float sY;
  float sZ;
  
  bool set;
};

bool compassSetup(bool master);
void printCompassSensorDetails();
float getCompassHeading(struct DistortionValues *dv);
sensors_event_t getCompassEvent();
void disableMPU6050();
struct DistortionValues getDistortionValues(struct MinMaxVector *mmv);
void updateMinMaxVector(struct MinMaxVector *mmv, sensors_event_t *event);


#endif // COMPASS_H_
