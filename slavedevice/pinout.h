#ifndef PINOUT_H_
#define PINOUT_H_

// Motor pins
const int MOTOR_PWM_1 = 19; // OUTPUT
const int MOTOR_PWM_1_CHANNEL = 0;
const int MOTOR_PWM_2 = 18; // OUTPUT
const int MOTOR_PWM_2_CHANNEL = 1;
const int MOTOR_INA1 = 25; // OUTPUT
const int MOTOR_INB1 = 26; // OUTPUT
const int MOTOR_INA2 = 27; // OUTPUT
const int MOTOR_INB2 = 14; // OUTPUT

// Ultrasonic pins
const int TRIGPIN = 2; // INPUT
const int ECHOPIN = 5; // OUTPUT
const float obstacleDist = 150; // Minimum distance to obstacle in front [mm]

// Defines for robot states
#define DONE 4
#define DRIVE 3
#define TURN 2
#define BUSY 1
#define READY 0
#define CALIBRATE 5

#endif // PINOUT_H_
