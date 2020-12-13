#ifndef STEPPER_H
#define STEPPER_H

void stepperSetup();
void medUret();
void modUret();
void findAfstand(int x);
float findAfvigendeLaengde(float akselsVaerdi);
float findDrejeVinkel();
void resetVinkel();
void testSensor();

#endif
