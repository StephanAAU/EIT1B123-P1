#include <math.h>

#include <Wire.h>
#include <VL53L1X.h>

// Define pin connections & motor's steps per revolution
const int dirPin = 23;
const int stepPin = 15;
const int stepsPerRevolution = 400;
float minValue = 4000, stepLock = 0, stepLockGrader = 0, laengdeAfvigelse = 0, A = 0, B = 0, drejeKegleVinkel = 0, akselsVaerdi = 1290;

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
  digitalWrite(dirPin, LOW);

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
  digitalWrite(dirPin, HIGH);

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
    Serial.println("minValue er ");
    Serial.println(minValue);
  }
}

float findAfvigendeLaengde() {
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
    Serial.println("laengdeAfvigelse er ");
    Serial.println(laengdeAfvigelse);
    return laengdeAfvigelse;
  }
  else {
    return 0;
  }
}

float findDrejeVinkel () {
  Serial.println("stepLockGrader er ");
  Serial.println(stepLockGrader);
  drejeKegleVinkel = atan(B/A);
  drejeKegleVinkel = drejeKegleVinkel * 100;
  Serial.println("drejeKegleVinkel er ");
  Serial.println(drejeKegleVinkel);
  return drejeKegleVinkel;
}

void reset() {
  minValue = 4000;
  stepLock = 0;
  A = 0;
  B = 0;
}

void loop()
{
  medUret();
  modUret();
  findAfvigendeLaengde();
  findDrejeVinkel();
  reset();
}
