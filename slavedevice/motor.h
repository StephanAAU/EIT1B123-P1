#ifndef MOTOR_H_
#define MOTOR_H_


extern int Pwm1Value;
extern int Pwm2Value;

extern int turnStart;
extern int turnEnd;
extern bool turning;

extern int driveStart;
extern int driveEnd;
extern bool driving;

/*
 * Run DC motors indefintely.
 */
void motorsRun();

/*
 * Stop all motors.
 */
void stopMotors();

/*
 * Stop motor.
 */
void stopMotor(int motor);

/*
 * Drive motors for a specific length in cm.
 * verifyDrive() must be called frequently for better precision.
 */
void drive(int cm);

/*
 * Used together with drive(). Determines when to stop motors.
 */
void verifyDrive();

/*
 * Drive motors to turn. Use negative deg to turn in opposite direction.
 * verifyTurn() must be called frequently for better precision.
 */
void turn(int deg);

/*
 * Used together with turn(). Determines when to stop motors.
 */
void verifyTurn();

void PwmSetup();

void updatePWMValues();


#endif // MOTOR_H_
