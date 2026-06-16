#ifndef __BALANCE_H
#define __BALANCE_H

#include "stm32f10x.h"

#define BALANCE_LOOP_MS        5
#define BALANCE_PWM_LIMIT      7190
#define BALANCE_SAFE_ANGLE_X100 5500
#define BALANCE_SOFT_ANGLE_X100 600
#define BALANCE_RESCUE_ANGLE_X100 1000
#define BALANCE_STOP_NONE      0
#define BALANCE_STOP_ANGLE     1
#define BALANCE_STOP_USER      2
#define BALANCE_OUTPUT_SIGN    1

extern float Balance_Angle_Kp;
extern float Balance_Soft_Angle_Kp;
extern float Balance_Angle_Kd;
extern float Balance_Soft_Angle_Kd;
extern float Balance_Rescue_Kp;
extern float Balance_Speed_Kp;
extern float Balance_Rescue_Speed_Kp;
extern float Balance_Position_Kp;

extern u8 Balance_Enable;
extern u8 Balance_Stop_Reason;
extern int Balance_Output;
extern int Balance_Position;
extern int Balance_Speed_Filter;
extern int Balance_Angle_Rate_X100;

void Balance_Reset(void);
void Balance_Start(void);
void Balance_Stop(void);
int Balance_Update(int angle_x100, int *encoder);

#endif
