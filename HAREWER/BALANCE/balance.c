#include "balance.h"

float Balance_Angle_Kp = 3.3f;
float Balance_Soft_Angle_Kp = 3.4f;
float Balance_Angle_Kd = 34.0f;
float Balance_Soft_Angle_Kd = 34.0f;
float Balance_Rescue_Kp = 1.1f;
float Balance_Speed_Kp = 1.35f;
float Balance_Rescue_Speed_Kp = 0.75f;
float Balance_Position_Kp = 0.001f;

u8 Balance_Enable = 0;
u8 Balance_Stop_Reason = BALANCE_STOP_NONE;
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

static int Balance_Sign(int value)
{
	if(value > 0) return 1;
	if(value < 0) return -1;
	return 0;
}

void Balance_Reset(void)
{
	Balance_Output = 0;
	Balance_Position = 0;
	Balance_Speed_Filter = 0;
	Balance_Angle_Rate_X100 = 0;
	last_angle_x100 = 0;
	Balance_Stop_Reason = BALANCE_STOP_NONE;
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
	int raw_speed;
	float angle_pwm;
	float rescue_pwm;
	float speed_pwm;
	float position_pwm;
	float angle_kp;
	float angle_kd;
	float speed_kp;
	int min_output;
	int abs_angle;

	if(!Balance_Enable)
	{
		Balance_Output = 0;
		last_angle_x100 = angle_x100;
		return 0;
	}

	if(Balance_Abs(angle_x100) > BALANCE_SAFE_ANGLE_X100)
	{
		Balance_Stop_Reason = BALANCE_STOP_ANGLE;
		Balance_Stop();
		return 0;
	}

	raw_speed = (encoder[0] + encoder[1] + encoder[2] + encoder[3]) / 4;
	speed = raw_speed * 5 / BALANCE_LOOP_MS;
	Balance_Position += speed;
	Balance_Position = Balance_Limit(Balance_Position, 200000);

	Balance_Speed_Filter = (Balance_Speed_Filter + speed) / 2;
	Balance_Angle_Rate_X100 = (angle_x100 - last_angle_x100) * 5 / BALANCE_LOOP_MS;
	last_angle_x100 = angle_x100;

	abs_angle = Balance_Abs(angle_x100);
	if(abs_angle < BALANCE_SOFT_ANGLE_X100)
	{
		angle_kp = Balance_Soft_Angle_Kp;
		angle_kd = Balance_Soft_Angle_Kd;
	}
	else
	{
		angle_kp = Balance_Angle_Kp;
		angle_kd = Balance_Angle_Kd;
	}

	angle_pwm = angle_kp * angle_x100 + angle_kd * Balance_Angle_Rate_X100;
	rescue_pwm = 0;
	if(abs_angle > BALANCE_RESCUE_ANGLE_X100)
	{
		rescue_pwm = Balance_Rescue_Kp * (abs_angle - BALANCE_RESCUE_ANGLE_X100) * Balance_Sign(angle_x100);
	}
	speed_kp = Balance_Speed_Kp;
	if(abs_angle > BALANCE_RESCUE_ANGLE_X100)
	{
		speed_kp = Balance_Rescue_Speed_Kp;
	}
	speed_pwm = speed_kp * Balance_Speed_Filter;
	position_pwm = Balance_Position_Kp * Balance_Position;

	Balance_Output = BALANCE_OUTPUT_SIGN * (int)(angle_pwm + rescue_pwm - speed_pwm - position_pwm);
	Balance_Output = Balance_Limit(Balance_Output, BALANCE_PWM_LIMIT);
	if(abs_angle > BALANCE_START_ANGLE_X100 && Balance_Output != 0 && Balance_Abs(Balance_Output) < BALANCE_MIN_OUTPUT)
	{
		min_output = BALANCE_MIN_OUTPUT;
		if(abs_angle < BALANCE_SOFT_ANGLE_X100)
		{
			min_output = BALANCE_MIN_OUTPUT * (abs_angle - BALANCE_START_ANGLE_X100) / (BALANCE_SOFT_ANGLE_X100 - BALANCE_START_ANGLE_X100);
		}
		if(Balance_Abs(Balance_Output) < min_output)
		{
			Balance_Output = min_output * Balance_Sign(Balance_Output);
		}
	}

	return Balance_Output;
}
