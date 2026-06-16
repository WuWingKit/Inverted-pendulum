#ifndef __KEY_H
#define __KEY_H

#include "sys.h"
#include "delay.h"

/*
 * KEY1/PA0 is not used here because PA0 is TIM5_CH1 PWM for motor A.
 * KEY2/PC8 and KEY3/PC9 are active-low keys.
 */
#define KEY_NONE        0
#define KEY_ZERO_PRESS  2
#define KEY_AUX_PRESS   3

void KEY_Init(void);
u8 KEY_Scan(void);

#endif
