#include "key.h"

#define KEY2_VAL  GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8)
#define KEY3_VAL  GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9)

void KEY_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

u8 KEY_Scan(void)
{
	static u8 key2_state = 1;
	static u8 key3_state = 1;
	static u8 key2_count = 0;
	static u8 key3_count = 0;
	u8 key2_now = KEY2_VAL;
	u8 key3_now = KEY3_VAL;

	if(key2_now == key2_state)
		key2_count = 0;
	else if(++key2_count >= 3)
	{
		key2_state = key2_now;
		key2_count = 0;
		if(key2_state == 0) return KEY_ZERO_PRESS;
	}

	if(key3_now == key3_state)
		key3_count = 0;
	else if(++key3_count >= 3)
	{
		key3_state = key3_now;
		key3_count = 0;
		if(key3_state == 0) return KEY_AUX_PRESS;
	}

	return KEY_NONE;
}
