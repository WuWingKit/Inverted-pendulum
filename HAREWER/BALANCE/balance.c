#include "balance.h"

float Balance_Angle_Kp = 4.2f;
float Balance_Angle_Kd = 48.0f;
float Balance_Rescue_Kp = 2.4f;
float Balance_Speed_Kp = 1.10f;
float Balance_Rescue_Speed_Kp = 0.30f;
float Balance_Fall_Rate_Kd = 58.0f;
float Balance_Fall_Speed_Kp = 0.04f;
float Balance_Return_Speed_Kp = 2.20f;
float Balance_Position_Kp = 0.005f;
float Balance_Drift_Position_Kp = 0.012f;

u8 Balance_Enable = 0;
u8 Balance_Stop_Reason = BALANCE_STOP_NONE;
int Balance_Output = 0;
int Balance_Position = 0;
int Balance_Speed_Filter = 0;
int Balance_Angle_Rate_X100 = 0;
int Balance_Angle_Bias_X100 = 0;

static int last_angle_x100 = 0;
static int last_raw_angle_x100 = 0;
static int kick_ticks = 0;
static int recover_ticks = 0;
static int kick_cooldown_ticks = 0;
static int kick_direction = 0;
static float angle_bias_x100 = 0.0f;

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
	Balance_Angle_Bias_X100 = 0;
	last_angle_x100 = 0;
	last_raw_angle_x100 = 0;
	kick_ticks = 0;
	recover_ticks = 0;
	kick_cooldown_ticks = 0;
	kick_direction = 0;
	angle_bias_x100 = 0.0f;
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
	int falling_away;
	int drifting;
	int kick_ready;
	int kicking_now;
	int min_output;
	int fall_min_output;
	int raw_angle_x100;
	int raw_angle_rate_x100;
	int abs_angle;

	if(!Balance_Enable)
	{
		Balance_Output = 0;
		last_angle_x100 = angle_x100;
		last_raw_angle_x100 = angle_x100;
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
	raw_angle_x100 = angle_x100;
	raw_angle_rate_x100 = (raw_angle_x100 - last_raw_angle_x100) * 5 / BALANCE_LOOP_MS;

	if(Balance_Abs(raw_angle_x100) < BALANCE_BIAS_ANGLE_X100 &&
	   Balance_Abs(raw_angle_rate_x100) < BALANCE_BIAS_RATE_X100 &&
	   Balance_Abs(Balance_Speed_Filter) < BALANCE_BIAS_SPEED &&
	   Balance_Abs(Balance_Position) < BALANCE_BIAS_POSITION)
	{
		angle_bias_x100 += 0.002f * ((float)raw_angle_x100 - angle_bias_x100);
		if(angle_bias_x100 > BALANCE_BIAS_LIMIT_X100) angle_bias_x100 = BALANCE_BIAS_LIMIT_X100;
		if(angle_bias_x100 < -BALANCE_BIAS_LIMIT_X100) angle_bias_x100 = -BALANCE_BIAS_LIMIT_X100;
		Balance_Angle_Bias_X100 = (int)angle_bias_x100;
	}

	angle_x100 = raw_angle_x100 - Balance_Angle_Bias_X100;
	Balance_Angle_Rate_X100 = (angle_x100 - last_angle_x100) * 5 / BALANCE_LOOP_MS;
	last_raw_angle_x100 = raw_angle_x100;
	last_angle_x100 = angle_x100;

	abs_angle = Balance_Abs(angle_x100);
	falling_away = (angle_x100 != 0 && Balance_Angle_Rate_X100 != 0 &&
	               Balance_Sign(angle_x100) == Balance_Sign(Balance_Angle_Rate_X100));
	drifting = (Balance_Abs(Balance_Speed_Filter) > BALANCE_DRIFT_SPEED ||
	           Balance_Abs(Balance_Position) > BALANCE_DRIFT_POSITION);
	if(kick_cooldown_ticks > 0) kick_cooldown_ticks--;
	if(abs_angle < BALANCE_HOLD_ANGLE_X100 && Balance_Abs(Balance_Angle_Rate_X100) < BALANCE_HOLD_RATE_X100)
	{
		Balance_Output = 0;
		Balance_Position = 0;
		return 0;
	}

	kick_ready = drifting && falling_away &&
		abs_angle > BALANCE_KICK_ANGLE_X100 &&
		Balance_Abs(Balance_Speed_Filter) > BALANCE_KICK_SPEED &&
		kick_ticks == 0 && recover_ticks == 0 && kick_cooldown_ticks == 0;
	if(kick_ready)
	{
		kick_direction = Balance_Sign(angle_x100);
		kick_ticks = BALANCE_KICK_TICKS;
	}

	angle_kp = Balance_Angle_Kp;
	angle_kd = Balance_Angle_Kd;

	angle_pwm = angle_kp * angle_x100 + angle_kd * Balance_Angle_Rate_X100;
	if(falling_away)
	{
		angle_pwm += Balance_Fall_Rate_Kd * Balance_Angle_Rate_X100;
	}

	rescue_pwm = 0;
	if(abs_angle > BALANCE_RESCUE_ANGLE_X100)
	{
		rescue_pwm = Balance_Rescue_Kp * (abs_angle - BALANCE_RESCUE_ANGLE_X100) * Balance_Sign(angle_x100);
	}
	speed_kp = Balance_Speed_Kp;
	if(falling_away)
	{
		speed_kp = Balance_Fall_Speed_Kp;
	}
	else if(abs_angle > BALANCE_RESCUE_ANGLE_X100)
	{
		speed_kp = Balance_Rescue_Speed_Kp;
	}
	if(drifting)
	{
		speed_kp = Balance_Return_Speed_Kp;
	}
	if(recover_ticks > 0)
	{
		speed_kp = BALANCE_RECOVER_SPEED_KP;
	}
	speed_pwm = speed_kp * Balance_Speed_Filter;
	position_pwm = (drifting ? Balance_Drift_Position_Kp : Balance_Position_Kp) * Balance_Position;

	Balance_Output = BALANCE_OUTPUT_SIGN * (int)(angle_pwm + rescue_pwm - speed_pwm - position_pwm);
	kicking_now = (kick_ticks > 0);
	if(kicking_now)
	{
		Balance_Output = BALANCE_KICK_PWM * kick_direction;
		kick_ticks--;
		if(kick_ticks == 0)
		{
			recover_ticks = BALANCE_RECOVER_TICKS;
			kick_cooldown_ticks = BALANCE_KICK_COOLDOWN_TICKS;
		}
	}
	else if(recover_ticks > 0)
	{
		recover_ticks--;
	}
	if(!kicking_now && drifting && Balance_Position != 0 &&
	   Balance_Sign(Balance_Output) == Balance_Sign(Balance_Position) &&
	   Balance_Abs(Balance_Output) > BALANCE_DRIFT_PUSH_LIMIT)
	{
		Balance_Output = BALANCE_DRIFT_PUSH_LIMIT * Balance_Sign(Balance_Output);
	}
	Balance_Output = Balance_Limit(Balance_Output, BALANCE_PWM_LIMIT);
	if(!drifting && falling_away && abs_angle > BALANCE_START_ANGLE_X100 && Balance_Output != 0 && Balance_Abs(Balance_Output) < BALANCE_MIN_OUTPUT)
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
	if(!drifting && falling_away && abs_angle > BALANCE_FALL_START_X100 && Balance_Output != 0)
	{
		fall_min_output = BALANCE_FALL_MIN_OUTPUT;
		if(abs_angle < BALANCE_RESCUE_ANGLE_X100 && BALANCE_RESCUE_ANGLE_X100 > BALANCE_FALL_START_X100)
		{
			fall_min_output = BALANCE_MIN_OUTPUT +
				(BALANCE_FALL_MIN_OUTPUT - BALANCE_MIN_OUTPUT) *
				(abs_angle - BALANCE_FALL_START_X100) /
				(BALANCE_RESCUE_ANGLE_X100 - BALANCE_FALL_START_X100);
		}
		if(Balance_Abs(Balance_Output) < fall_min_output)
		{
			Balance_Output = fall_min_output * Balance_Sign(Balance_Output);
		}
	}
	return Balance_Output;
}
