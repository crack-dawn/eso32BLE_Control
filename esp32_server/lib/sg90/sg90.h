

#ifndef _SG90_H
#define _SG90_H

#include <Arduino.h>

int calculatePWM(int degree);
void Sg90Init();
void Sg90Ctrl(int x);

#endif