#ifndef MOTOR_H_
#define MOTOR_H_

const int PWM_RESOLUTION = 8;

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
bool verifyDrive();

/*
 * Drive motors to turn. Use negative deg to turn in opposite direction.
 * verifyTurn() must be called frequently for better precision.
 */
void turn(int deg);

/*
 * Used together with turn(). Determines when to stop motors.
 */
bool verifyTurn();

void motorSetup();


/*
 * Update duty cycle for motor 1 and motor 2. Use -1 to leave unchanged.
 */
void updateMotorDutyCycle(int motor1DutyCycle, int motor2DutyCycle);


#endif // MOTOR_H_
