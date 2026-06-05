#include "stm32f10x.h"

#include "delay.h"
#include "gpio.h"
#include "moto.h"
#include "pwm.h"
#include "adc.h"
#include "usart.h"
#include "encoder.h"
#include "lcd.h"
#include "lcd_init.h"

 /**************************************************************************
WHEELTEC D24Ademo - 霍尔编码器四轮电机控制
增加LCD显示实时速度，用于调试闭环控制
**************************************************************************/

int TargetVelocity=500;

// LCD显示缓冲
void LCD_Show_Debug(int *enc, int *pwm, float vcc)
{
	// 第1行: 目标速度 + 电压
	LCD_ShowString(0,  0, (u8*)"T:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(16, 0, TargetVelocity, 4, GREEN, BLACK, 16);
	LCD_ShowString(48, 0, (u8*)" V:", WHITE, BLACK, 16, 0);
	LCD_ShowFloatNum1(72, 0, vcc, 4, GREEN, BLACK, 16);

	// 第2行: A电机
	LCD_ShowString(0,  16, (u8*)"A:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(16, 16, enc[0], 5, YELLOW, BLACK, 16);
	LCD_ShowString(56, 16, (u8*)" P:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(72, 16, pwm[0], 5, CYAN, BLACK, 16);

	// 第3行: B电机
	LCD_ShowString(0,  32, (u8*)"B:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(16, 32, enc[1], 5, YELLOW, BLACK, 16);
	LCD_ShowString(56, 32, (u8*)" P:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(72, 32, pwm[1], 5, CYAN, BLACK, 16);

	// 第4行: C电机
	LCD_ShowString(0,  48, (u8*)"C:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(16, 48, enc[2], 5, YELLOW, BLACK, 16);
	LCD_ShowString(56, 48, (u8*)" P:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(72, 48, pwm[2], 5, CYAN, BLACK, 16);

	// 第5行: D电机
	LCD_ShowString(0,  64, (u8*)"D:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(16, 64, enc[3], 5, YELLOW, BLACK, 16);
	LCD_ShowString(56, 64, (u8*)" P:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(72, 64, pwm[3], 5, CYAN, BLACK, 16);

	// 第6行: 控制模式
	LCD_ShowString(0,  80, (u8*)"Mode:PI Closed ", WHITE, BLACK, 16, 0);

	// 第7行: PID参数
	LCD_ShowString(0,  96, (u8*)"Kp:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(24, 96, (int)Velcity_Kp, 2, GREEN, BLACK, 16);
	LCD_ShowString(40, 96, (u8*)" Ki:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(64, 96, (int)Velcity_Ki, 2, GREEN, BLACK, 16);

	// 第8行: 分隔线
	LCD_ShowString(0, 112, (u8*)"================", GRAY, BLACK, 16, 0);
}

int main(void)
{
	int encoder[4];
	int pwm[4];
	u16 adcx;
	float vcc;

	SystemInit();
	delay_init();
	Gpio_Init();
	uart_init(115200);
	adc_Init();
	PWM_Int(7199,0);
	Encoder_Init_Tim8();
	Encoder_Init_Tim2();
	Encoder_Init_Tim3();
	Encoder_Init_Soft(); // D电机编码器→PB14/PB15软件解码

	// 初始化LCD
	LCD_Init();
	LCD_Fill(0, 0, 128, 128, BLACK);

	// 开机画面
	LCD_ShowString(0, 48, (u8*)"Motor Debug", GREEN, BLACK, 16, 0);
	LCD_ShowString(0, 64, (u8*)"Loading...", GREEN, BLACK, 16, 0);
	delay_ms(1000);
	LCD_Fill(0, 0, 128, 128, BLACK);

	while(1)
	{
		// 读取电池电压
		adcx = Get_adc_Average(ADC_Channel_5, 10);
		vcc = (float)adcx * (3.3 * 11 / 4096);

		// 读取编码器 (D电机用PB14/PB15软件解码)
		Encoder_Soft_Poll(); // 轮询PB14/PB15正交解码
		encoder[0] = Read_Encoder(8);
		encoder[1] = Read_Encoder(2);
		encoder[2] = Read_Encoder(3);
		encoder[3] = Read_Encoder(4); // 软件编码器

		// PI闭环控制
		pwm[0] = Velocity_A(TargetVelocity, encoder[0]);
		pwm[1] = Velocity_B(TargetVelocity, encoder[1]);
		pwm[2] = Velocity_C(TargetVelocity, encoder[2]);
		pwm[3] = Velocity_D(TargetVelocity, encoder[3]);
		Set_PWM(pwm[0], pwm[1], pwm[2], pwm[3]);

		// LCD显示调试信息
		LCD_Show_Debug(encoder, pwm, vcc);

		// 串口输出
		printf("T=%d V=%.2f A=%d/%d B=%d/%d C=%d/%d D=%d/%d\r\n",
			TargetVelocity, vcc,
			encoder[0], pwm[0], encoder[1], pwm[1],
			encoder[2], pwm[2], encoder[3], pwm[3]);

		delay_ms(50);
	}
}
