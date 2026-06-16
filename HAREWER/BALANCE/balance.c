#include "balance.h"

float Balance_Angle_Kp = 3.8f;
float Balance_Angle_Kd = 24.0f;
float Balance_Speed_Kp = 0.55f;
float Balance_Position_Kp = 0.001f;

u8 Balance_Enable = 0;
int Balance_Output = 0;
int Balance_Position = 0;
int Balance_Speed_Filter = 0;
int Balance_Angle_Rate_X100 = 0;

static int last_angle_x100 = 0;

static int Balance_Limit(int value, int limit)
{
	if(value > limit) return limit;
	if(value < -limit) return -limit;
	return value;
}

static int Balance_Abs(int value)
{
	return value < 0 ? -value : value;
}

void Balance_Reset(void)
{
	Balance_Output = 0;
	Balance_Position = 0;
	Balance_Speed_Filter = 0;
	Balance_Angle_Rate_X100 = 0;
	last_angle_x100 = 0;
}

void Balance_Start(void)
{
	Balance_Reset();
	Balance_Enable = 1;
}

void Balance_Stop(void)
{
	Balance_Enable = 0;
	Balance_Output = 0;
}

int Balance_Update(int angle_x100, int *encoder)
{
	int speed;
	float angle_pwm;
	float speed_pwm;
	float position_pwm;

	if(!Balance_Enable)
	{
		Balance_Output = 0;
		last_angle_x100 = angle_x100;
		return 0;
	}

	if(Balance_Abs(angle_x100) > BALANCE_SAFE_ANGLE_X100)
	{
		Balance_Stop();
		return 0;
	}

	speed = (encoder[0] + encoder[1] + encoder[2] + encoder[3]) / 4;
	Balance_Position += speed;
	Balance_Position = Balance_Limit(Balance_Position, 200000);

	Balance_Speed_Filter = (Balance_Speed_Filter * 7 + speed * 3) / 10;
	Balance_Angle_Rate_X100 = angle_x100 - last_angle_x100;
	last_angle_x100 = angle_x100;

	angle_pwm = Balance_Angle_Kp * angle_x100 + Balance_Angle_Kd * Balance_Angle_Rate_X100;
	speed_pwm = Balance_Speed_Kp * Balance_Speed_Filter;
	position_pwm = Balance_Position_Kp * Balance_Position;

	Balance_Output = BALANCE_OUTPUT_SIGN * (int)(angle_pwm - speed_pwm - position_pwm);
	Balance_Output = Balance_Limit(Balance_Output, BALANCE_PWM_LIMIT);

	return Balance_Output;
}
