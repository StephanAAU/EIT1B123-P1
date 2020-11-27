#include <Arduino.h>

const int PINGPIN = 23; // Trigger Pin of Ultrasonic Sensor
const int ECHOPIN = 22; // Echo Pin of Ultrasonic Sensor
const int INPUTPIN = 21; // Input pin for push button.

void setup() {
  Serial.begin(115200); // Starting Serial Terminal
  pinMode(PINGPIN, OUTPUT);{}
  pinMode(ECHOPIN, INPUT);
  pinMode(INPUTPIN, INPUT_PULLUP);
  pinMode(BUILTIN_LED, OUTPUT);
}

long microsecondsToCentimeters(long microseconds) {
   return microseconds / 29 / 2;
}

long measureDistance()
{
  long duration;
  digitalWrite(PINGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(PINGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(PINGPIN, LOW);
  duration = pulseIn(ECHOPIN, HIGH);
  return(microsecondsToCentimeters(duration));
}

long avgDistance() {
  long measurements;
  for (int x = 0; x < 5; x++)
  {
    measurements += measureDistance();
  }
  return measurements / 5;
}

void loop() {  
  long x = avgDistance();
  Serial.print(x);
  Serial.println(" cm");
  delay(100);
}
