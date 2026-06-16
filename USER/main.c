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
#include "key.h"

 /**************************************************************************
WHEELTEC D24Ademo - 霍尔编码器四轮电机控制
LCD实时显示四路电机速度与PWM调试数据
**************************************************************************/

int TargetVelocity=0;
u16 angle_adc;          // WDD35D4角度传感器 ADC原始值
int zero_offset = 2048; // 竖直时ADC零点，默认中间值
int angle_x100;         // 角度*100（定点数，避免浮点）

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

	// 第3行:                                                                                                                                                                                                                                B电机
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

	// 第7行: WDD35D4角度 (带符号, 2位小数)
	{
		int abs_fp = angle_x100 < 0 ? -angle_x100 : angle_x100;
		LCD_ShowString(0,  96, (u8*)"Ang:", WHITE, BLACK, 16, 0);
		LCD_ShowIntNum(32, 96, angle_x100 / 100, 4, GREEN, BLACK, 16);
		LCD_ShowString(64, 96, (u8*)".", WHITE, BLACK, 16, 0);
		LCD_ShowIntNum(72, 96, abs_fp % 100, 2, GREEN, BLACK, 16);
	}

	// 第8行: ADC原始值 + 零点偏移 (调试用)
	LCD_ShowString(0, 112, (u8*)"A:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(16, 112, angle_adc, 4, YELLOW, BLACK, 16);
	LCD_ShowString(48, 112, (u8*)"O:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(64, 112, zero_offset, 4, CYAN, BLACK, 16);
}

int main(void)
{
	int encoder[4];
	int pwm[4];
	u16 adcx;
	float vcc;
	u8 key;

	SystemInit();
	delay_init();
	Gpio_Init();
	KEY_Init();
	uart_init(115200);
	adc_Init();
	PWM_Int(7199,0);
	Encoder_Init_Tim8();
	Encoder_Init_Tim2();
	Encoder_Init_Tim3();
	Encoder_Init_Soft(); // D电机编码器→PA8/PA4软件解码
	Encoder_Timer_Init(); // 启动1ms定时器轮询D编码器

	// 初始化LCD
	LCD_Init();
	LCD_Fill(0, 0, 128, 128, BLACK);

	// 启动提示
	printf("WDD35D4 Angle Sensor Ready. Press KEY2 or send 'z'+CR+LF to zero.\r\n");


	while(1)
	{
		// 读取电池电压
		adcx = Get_adc_Average(ADC_Channel_5, 10);
		vcc = (float)adcx * (3.3 * 11 / 4096);

		// 读取WDD35D4角度传感器 (PC4 = ADC1_CH14)
		angle_adc = Get_adc(ADC_Channel_14);
		key = KEY_Scan();
		if(key == KEY_ZERO_PRESS)
		{
			zero_offset = angle_adc;
			printf("Zero set by KEY2! ADC=%d\r\n", zero_offset);
		}
		angle_x100 = ((int)angle_adc - zero_offset) * 34000 / 4096; // 角度*100

		// 串口命令: 'z' = 调零 (杆子竖直时发送)
		if(USART_RX_STA & 0x8000)
		{
			u8 len = USART_RX_STA & 0x3FFF;
			// 回显收到的内容，方便调试
			printf("CMD[%d]:", len);
			{ u8 i; for(i=0;i<len;i++) printf("%02X ", USART_RX_BUF[i]); }
			printf("\r\n");
			if(len >= 1 && (USART_RX_BUF[0] == 'z' || USART_RX_BUF[0] == 'Z'))
			{
				zero_offset = angle_adc;
				printf("Zero set! ADC=%d\r\n", zero_offset);
			}
			USART_RX_STA = 0;
		}

		// 读取编码器 (D电机用PA8/PA4软件解码)
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
		printf("T=%d V=%.2f A=%d/%d B=%d/%d C=%d/%d D=%d/%d Ang=%d.%02d ADC=%d Off=%d\r\n",
			TargetVelocity, vcc,
			encoder[0], pwm[0], encoder[1], pwm[1],
			encoder[2], pwm[2], encoder[3], pwm[3],
			angle_x100/100, (angle_x100<0?-angle_x100:angle_x100)%100,
			angle_adc, zero_offset);

		delay_ms(50);
	}
}
