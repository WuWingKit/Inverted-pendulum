#ifndef __BALANCE_H
#define __BALANCE_H

#include "stm32f10x.h"

#define BALANCE_LOOP_MS        2
#define BALANCE_PWM_LIMIT      6200
#define BALANCE_SAFE_ANGLE_X100 4500
#define BALANCE_SOFT_ANGLE_X100 600
#define BALANCE_RESCUE_ANGLE_X100 500
#define BALANCE_START_ANGLE_X100 60
#define BALANCE_HOLD_ANGLE_X100 35
#define BALANCE_HOLD_RATE_X100 120
#define BALANCE_MIN_OUTPUT      480
#define BALANCE_FALL_START_X100 120
#define BALANCE_FALL_MIN_OUTPUT 1450
#define BALANCE_CENTER_CAPTURE_X100 60
#define BALANCE_RETURN_BRAKE_X100 250
#define BALANCE_BIAS_ANGLE_X100 450
#define BALANCE_BIAS_RATE_X100 260
#define BALANCE_BIAS_SPEED      500
#define BALANCE_BIAS_POSITION   8000
#define BALANCE_BIAS_LIMIT_X100 180
#define BALANCE_DRIFT_SPEED     900
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
extern float Balance_Fall_Rate_Kd;
extern float Balance_Fall_Speed_Kp;
extern float Balance_Return_Speed_Kp;
extern float Balance_Carry_Speed_Kp;
extern float Balance_Carry_Angle_Kd;
extern float Balance_Position_Kp;

extern u8 Balance_Enable;
extern u8 Balance_Stop_Reason;
extern int Balance_Output;
extern int Balance_Position;
extern int Balance_Speed_Filter;
extern int Balance_Angle_Rate_X100;
extern int Balance_Angle_Bias_X100;

void Balance_Reset(void);
void Balance_Start(void);
void Balance_Stop(void);
int Balance_Update(int angle_x100, int *encoder);

#endif
