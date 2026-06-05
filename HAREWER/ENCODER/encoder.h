#ifndef _ENCODER_H
#define _ENCODER_H

#include "stm32f10x.h"

void Encoder_Init_Tim2(void);
void Encoder_Init_Tim4(void);
void Encoder_Timer_Init(void);
void Encoder_Init_Soft(void);
void Encoder_Soft_Poll(void);
int Read_Encoder(u8 TIMX);
void Encoder_Init_Tim3(void);
void Encoder_Init_Tim8(void);

extern volatile int16_t soft_enc4_cnt;
#endif
