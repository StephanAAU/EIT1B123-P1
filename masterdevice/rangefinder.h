#ifndef RANGEFINDER_H
#define RANGEFINDER_H

void afstandTest();
void findAfstand(int x, float *minValue, float *stepLock);
float beregnAfstandTilKegle(float x);
void initRangefinders();
float afstandTilLift();
float afstandTilKegle();

#endif
