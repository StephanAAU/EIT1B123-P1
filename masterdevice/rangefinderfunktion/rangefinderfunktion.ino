#include <math.h>

#include <Wire.h>
#include <VL53L1X.h>

// Define pin connections & motor's steps per revolution
const int dirPin = 23;
const int stepPin = 15;
const int stepsPerRevolution = 400;
float minValue = 4000, stepLock = 0, afstandAfvigelse = 0, radianAfvigelse = 0, laengdeAfvigelse = 0, drejeKegleVinkel = 0;

VL53L1X sensor;
float y;
float afstandTilKegle(float x) {
    return (tan(0.5235988)*x);
  }
  
void setup()
{
  Serial.begin(115200);
  // Declare pins as Outputs
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  Wire.begin();
  Wire.setClock(400000); // use 400 kHz I2C

  sensor.setTimeout(500);
  if (!sensor.init())
  {
    Serial.println("Failed to detect and initialize sensor!");
    while (1);
  }

  sensor.setDistanceMode(VL53L1X::Long);
  sensor.setMeasurementTimingBudget(20000);
  sensor.startContinuous(50);
}

void medUret() {
  // Set motor direction clockwise
  digitalWrite(dirPin, HIGH);

  // Spin motor speed
  for(int x = 0; x < stepsPerRevolution*0.75; x++)
  {
    findAfstand(x);
    digitalWrite(stepPin, HIGH);
    digitalWrite(stepPin, LOW);
  }
}

void modUret() {
  // Set motor direction counterclockwise
  digitalWrite(dirPin, LOW);

  // Spin motor speed
  for(int x = 300; x > 0; x--)
  {
    findAfstand(x);
    digitalWrite(stepPin, HIGH);
    digitalWrite(stepPin, LOW);
  }
}

void findAfstand(int x) {
  y = sensor.read();
  if (y < minValue) {
    minValue = y;
    stepLock = x;
  }
}

float findAfvigendeLaengde () {
  float favPosition = 150 * 0.9;
  stepLock = stepLock * 0.9;
  if (stepLock > favPosition + 3*0.9 or stepLock < favPosition - 3*0.9) {
    radianAfvigelse = (favPosition - stepLock) * 3.14/180;
    laengdeAfvigelse = (minValue*minValue + minValue*minValue) - (2 * minValue*minValue * cos(radianAfvigelse));
    laengdeAfvigelse = sqrt(laengdeAfvigelse);
    return laengdeAfvigelse;
  }
  else {
    return 0;
  }
}

float findDrejeVinkel () {
  float favPosition = 150 * 0.9;
  stepLock = stepLock * 0.9;
  drejeKegleVinkel = (180 - (favPosition - stepLock)) / 2;
  return drejeKegleVinkel;
}

float checkLaengdeTilKegle(float AkselsVaerdi) {
  if (minValue > AkselsVaerdi + 100 or minValue < AkselsVaerdi - 100) {
    afstandAfvigelse = AkselsVaerdi - minValue;
    return afstandAfvigelse;
  }
  else {
    return 0;
  }
}

void reset() {
  minValue = 4000;
  stepLock = 0;
}

void loop()
{
  medUret();
  modUret();
  findAfvigendeLaengde();
  findDrejeVinkel();
  checkLaengdeTilKegle(float AkselsVaerdi);
  reset();
}
