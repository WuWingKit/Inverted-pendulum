#include "stm32f10x.h"

#include "delay.h"
#include "gpio.h"
#include "pwm.h"
#include "adc.h"
#include "usart.h"
#include "encoder.h"
#include "lcd.h"
#include "lcd_init.h"
#include "key.h"
#include "balance.h"

/**************************************************************************
WHEELTEC D24Ademo - four-wheel cart inverted pendulum control.
KEY2: zero WDD35D4 angle sensor and start balance.
KEY3: stop balance output.
**************************************************************************/

#define STARTUP_DIAGNOSTIC_MODE 0

int TargetVelocity = 0;
u16 angle_adc;
int zero_offset = 2048;
int angle_x100;
int encoder[4] = {0, 0, 0, 0};
int pwm[4] = {0, 0, 0, 0};
int balance_pwm = 0;
u16 battery_adc;
float vcc = 0;
volatile u8 lcd_update_flag = 0;
volatile u8 serial_update_flag = 0;
volatile u8 key_update_flag = 0;

static void Startup_Mark(u8 line, char *text)
{
	printf("BOOT: %s\r\n", text);
	LCD_ShowString(0, line, (u8*)text, GREEN, BLACK, 16, 0);
}

static void Balance_Timer_Set(u8 enable)
{
	if(enable) TIM_Cmd(TIM1, ENABLE);
	else TIM_Cmd(TIM1, DISABLE);
}

static void Balance_Timer_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	TIM_TimeBaseStructure.TIM_Prescaler = 7200 - 1;  // 72MHz / 7200 = 10kHz
	TIM_TimeBaseStructure.TIM_Period = BALANCE_LOOP_MS * 10 - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	TIM_ClearFlag(TIM1, TIM_FLAG_Update);
	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIM1, ENABLE);
}

static void Calibrate_Angle_Zero(void)
{
	Balance_Timer_Set(0);
	Balance_Stop();
	Set_PWM(0, 0, 0, 0);
	zero_offset = Get_adc_Average(ADC_Channel_14, 50);
	angle_adc = zero_offset;
	angle_x100 = 0;
	Balance_Timer_Set(1);
}

static void LCD_Show_Debug(int *enc, int *pwm, float vcc)
{
	LCD_ShowString(0,  0, (u8*)"T:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(16, 0, TargetVelocity, 4, GREEN, BLACK, 16);
	LCD_ShowString(48, 0, (u8*)" V:", WHITE, BLACK, 16, 0);
	LCD_ShowFloatNum1(72, 0, vcc, 4, GREEN, BLACK, 16);

	LCD_ShowString(0,  16, (u8*)"A:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(16, 16, enc[0], 5, YELLOW, BLACK, 16);
	LCD_ShowString(56, 16, (u8*)" P:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(72, 16, pwm[0], 5, CYAN, BLACK, 16);

	LCD_ShowString(0,  32, (u8*)"B:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(16, 32, enc[1], 5, YELLOW, BLACK, 16);
	LCD_ShowString(56, 32, (u8*)" P:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(72, 32, pwm[1], 5, CYAN, BLACK, 16);

	LCD_ShowString(0,  48, (u8*)"C:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(16, 48, enc[2], 5, YELLOW, BLACK, 16);
	LCD_ShowString(56, 48, (u8*)" P:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(72, 48, pwm[2], 5, CYAN, BLACK, 16);

	LCD_ShowString(0,  64, (u8*)"D:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(16, 64, enc[3], 5, YELLOW, BLACK, 16);
	LCD_ShowString(56, 64, (u8*)" P:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(72, 64, pwm[3], 5, CYAN, BLACK, 16);

	if(Balance_Enable)
		LCD_ShowString(0,  80, (u8*)"Mode:BAL ON    ", WHITE, BLACK, 16, 0);
	else
		LCD_ShowString(0,  80, (u8*)"Mode:BAL STOP  ", WHITE, BLACK, 16, 0);

	{
		int abs_fp = angle_x100 < 0 ? -angle_x100 : angle_x100;
		LCD_ShowString(0,  96, (u8*)"Ang:", WHITE, BLACK, 16, 0);
		LCD_ShowIntNum(32, 96, angle_x100 / 100, 4, GREEN, BLACK, 16);
		LCD_ShowString(64, 96, (u8*)".", WHITE, BLACK, 16, 0);
		LCD_ShowIntNum(72, 96, abs_fp % 100, 2, GREEN, BLACK, 16);
	}

	LCD_ShowString(0, 112, (u8*)"A:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(16, 112, angle_adc, 4, YELLOW, BLACK, 16);
	LCD_ShowString(48, 112, (u8*)"O:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(64, 112, zero_offset, 4, CYAN, BLACK, 16);
}

static void Handle_Key(u8 key)
{
	if(key == KEY_ZERO_PRESS)
	{
		Calibrate_Angle_Zero();
		Balance_Start();
		printf("Zero set by KEY2, balance start! ADC=%d\r\n", zero_offset);
	}
	else if(key == KEY_AUX_PRESS)
	{
		Balance_Stop_Reason = BALANCE_STOP_USER;
		Balance_Stop();
		Set_PWM(0, 0, 0, 0);
		printf("Balance stopped by KEY3.\r\n");
	}
}

static void Handle_Serial_Command(void)
{
	if(USART_RX_STA & 0x8000)
	{
		u8 len = USART_RX_STA & 0x3FFF;
		if(len >= 1 && (USART_RX_BUF[0] == 'z' || USART_RX_BUF[0] == 'Z'))
		{
			Calibrate_Angle_Zero();
			Balance_Start();
			printf("Zero set by serial, balance start! ADC=%d\r\n", zero_offset);
		}
		else if(len >= 1 && (USART_RX_BUF[0] == 's' || USART_RX_BUF[0] == 'S'))
		{
			Balance_Stop_Reason = BALANCE_STOP_USER;
			Balance_Stop();
			Set_PWM(0, 0, 0, 0);
			printf("Balance stopped by serial.\r\n");
		}
		USART_RX_STA = 0;
	}
}

int main(void)
{
	SystemInit();
	delay_init();
	uart_init(115200);
	printf("BOOT: System/delay/uart OK\r\n");

	LCD_Init();
	LCD_Fill(0, 0, 128, 128, BLACK);
	Startup_Mark(0, "BOOT LCD OK");

	Gpio_Init();
	Startup_Mark(16, "GPIO OK");
	KEY_Init();
	Startup_Mark(32, "KEY OK");
	adc_Init();
	Startup_Mark(48, "ADC OK");
	PWM_Int(7199, 0);
	Set_PWM(0, 0, 0, 0);
	Startup_Mark(64, "PWM OK");
	Encoder_Init_Tim8();
	Encoder_Init_Tim2();
	Encoder_Init_Tim3();
	Encoder_Init_Soft();
#if STARTUP_DIAGNOSTIC_MODE
	Startup_Mark(80, "ENC TIMER SKIP");
#else
	Encoder_Timer_Init();
	Startup_Mark(80, "ENC TIMER OK");
#endif
	Balance_Timer_Init();

	printf("Cart balance ready. Hold pendulum vertical, press KEY2 to zero and start.\r\n");

	while(1)
	{
		if(key_update_flag)
		{
			key_update_flag = 0;
			Handle_Key(KEY_Scan());
		}
		Handle_Serial_Command();

		if(serial_update_flag)
		{
			serial_update_flag = 0;
			printf("BAL=%d Stop=%d V=%.2f A=%d B=%d C=%d D=%d PWM=%d Ang=%d.%02d Rate=%d SF=%d ADC=%d Off=%d\r\n",
				Balance_Enable, Balance_Stop_Reason, vcc,
				encoder[0], encoder[1], encoder[2], encoder[3],
				balance_pwm,
				angle_x100 / 100, (angle_x100 < 0 ? -angle_x100 : angle_x100) % 100,
				Balance_Angle_Rate_X100, Balance_Speed_Filter, angle_adc, zero_offset);
		}

		if(lcd_update_flag)
		{
			lcd_update_flag = 0;
			LCD_Show_Debug(encoder, pwm, vcc);
		}
	}
}

void TIM1_UP_IRQHandler(void)
{
	static u8 serial_tick = 0;
	static u8 lcd_tick = 0;
	static u8 vbat_tick = 0;

	if(TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
		key_update_flag = 1;

		angle_adc = Get_adc(ADC_Channel_14);
		angle_x100 = ((int)angle_adc - zero_offset) * 34000 / 4096;

		encoder[0] = Read_Encoder(8);
		encoder[1] = Read_Encoder(2);
		encoder[2] = Read_Encoder(3);
		encoder[3] = Read_Encoder(4);

		balance_pwm = Balance_Update(angle_x100, encoder);
		pwm[0] = balance_pwm;
		pwm[1] = balance_pwm;
		pwm[2] = balance_pwm;
		pwm[3] = balance_pwm;
		Set_PWM(pwm[0], pwm[1], pwm[2], pwm[3]);

		if(++vbat_tick >= 50)
		{
			vbat_tick = 0;
			battery_adc = Get_adc(ADC_Channel_5);
			vcc = (float)battery_adc * (3.3 * 11 / 4096);
		}

		if(++serial_tick >= 10)
		{
			serial_tick = 0;
			serial_update_flag = 1;
		}

		if(++lcd_tick >= 50)
		{
			lcd_tick = 0;
			lcd_update_flag = 1;
		}
	}
}
