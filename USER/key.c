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
	if(KEY2_VAL == 0)
	{
		delay_ms(20);
		if(KEY2_VAL == 0)
		{
			while(KEY2_VAL == 0);
			return KEY_ZERO_PRESS;
		}
	}

	if(KEY3_VAL == 0)
	{
		delay_ms(20);
		if(KEY3_VAL == 0)
		{
			while(KEY3_VAL == 0);
			return KEY_AUX_PRESS;
		}
	}

	return KEY_NONE;
}
