#ifndef __MOTO_H
#define	__MOTO_H

#include "stm32f10x.h"

extern float Velcity_Kp, Velcity_Ki, Velcity_Kd;

void moto(int mode);
int Velocity_A(int TargetVelocity, int CurrentVelocity);
int Velocity_B(int TargetVelocity, int CurrentVelocity);
int Velocity_C(int TargetVelocity, int CurrentVelocity);
int Velocity_D(int TargetVelocity, int CurrentVelocity);
#endif
