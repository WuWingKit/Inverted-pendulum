#ifndef __LCD_INIT_H
#define __LCD_INIT_H

#include "stm32f10x.h"

#define USE_HORIZONTAL 1  //0/1:竖屏 2/3:横屏

#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define LCD_W 128
#define LCD_H 128
#else
#define LCD_W 128
#define LCD_H 128
#endif

//LCD引脚定义 - PB4~PB9 (RES:PB6 DC:PB7)
#define LCD_SCLK_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_4)
#define LCD_SCLK_Set() GPIO_SetBits(GPIOB,GPIO_Pin_4)

#define LCD_MOSI_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_5)
#define LCD_MOSI_Set() GPIO_SetBits(GPIOB,GPIO_Pin_5)

#define LCD_RES_Clr()  GPIO_ResetBits(GPIOB,GPIO_Pin_6)
#define LCD_RES_Set()  GPIO_SetBits(GPIOB,GPIO_Pin_6)

#define LCD_DC_Clr()   GPIO_ResetBits(GPIOB,GPIO_Pin_7)
#define LCD_DC_Set()   GPIO_SetBits(GPIOB,GPIO_Pin_7)

#define LCD_CS_Clr()   GPIO_ResetBits(GPIOB,GPIO_Pin_8)
#define LCD_CS_Set()   GPIO_SetBits(GPIOB,GPIO_Pin_8)

#define LCD_BLK_Clr()  GPIO_ResetBits(GPIOB,GPIO_Pin_9)
#define LCD_BLK_Set()  GPIO_SetBits(GPIOB,GPIO_Pin_9)

void LCD_GPIO_Init(void);
void LCD_Writ_Bus(u8 dat);
void LCD_WR_DATA8(u8 dat);
void LCD_WR_DATA(u16 dat);
void LCD_WR_REG(u8 dat);
void LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2);
void LCD_Init(void);
#endif
