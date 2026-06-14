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
#include "filter.h"     // Kalman + complementary filter

 /**************************************************************************
WHEELTEC D24Ademo - 霍尔编码器四轮电机控制
LCD实时显示四路电机速度与PWM调试数据
**************************************************************************/

int TargetVelocity=300;

int   sensor_angle_raw;     // Raw ADC value from WDD35D4
float adc_zero_offset = 2048; // ADC zero-point offset (calibrate: sensor centered = ~mid ADC range)

// LCD display for WDD35D4 sensor data (all int to avoid float display bugs)
void LCD_Show_Angle(float vcc, int adc_raw)
{
	int angle_int; // angle * 100 for fixed-point display
	int vcc_int;   // voltage * 100

	angle_int = (int)((adc_raw - adc_zero_offset) * 34000.0 / 4096.0);
	vcc_int   = (int)(vcc * 100);

	// Row 1: Title
	LCD_ShowString(0,  0, (u8*)"WDD35D4 Test   ", WHITE, BLACK, 16, 0);

	// Row 2: Raw ADC value
	LCD_ShowString(0,  16, (u8*)"ADC:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(32, 16, adc_raw, 5, YELLOW, BLACK, 16);

	// Row 3: Angle as int * 100 (two decimal places)
	LCD_ShowString(0,  32, (u8*)"Ang:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(32, 32, angle_int / 100, 4, GREEN, BLACK, 16);
	LCD_ShowString(64, 32, (u8*)".", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(72, 32, (angle_int < 0 ? -angle_int : angle_int) % 100, 2, GREEN, BLACK, 16);

	// Row 4: Battery voltage as int * 100
	LCD_ShowString(0,  48, (u8*)"V:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(16, 48, vcc_int / 100, 2, GREEN, BLACK, 16);
	LCD_ShowString(32, 48, (u8*)".", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(40, 48, vcc_int % 100, 2, GREEN, BLACK, 16);
	LCD_ShowString(56, 48, (u8*)"V", WHITE, BLACK, 16, 0);

	// Row 5: Sensor status
	LCD_ShowString(0,  64, (u8*)"Sensor: PC4     ", WHITE, BLACK, 16, 0);

	// Row 6: Motor status
	LCD_ShowString(0,  80, (u8*)"Motors: OFF     ", RED, BLACK, 16, 0);
}

int main(void)
{
	u16 adcx;
	float vcc;

	SystemInit();
	delay_init();
	Gpio_Init();
	uart_init(115200);
	adc_Init();
	// PWM_Int(7199,0);               // DISABLED - motors off for sensor test
	// Encoder_Init_Tim8();           // DISABLED
	// Encoder_Init_Tim2();           // DISABLED
	// Encoder_Init_Tim3();           // DISABLED
	// Encoder_Init_Soft();           // DISABLED
	// Encoder_Timer_Init();          // DISABLED

	// 初始化LCD
	LCD_Init();
	LCD_Fill(0, 0, 128, 128, BLACK);

	// Startup message
	printf("WDD35D4 Angle Sensor Test\r\n");
	printf("PC4 = ADC1_CH14\r\n");

	while(1)
	{
		// Read battery voltage (PA5)
		adcx = Get_adc_Average(ADC_Channel_5, 10);
		vcc = (float)adcx * (3.3 * 11 / 4096);

		// Read WDD35D4 angle sensor (PC4 = ADC1_CH14)
		sensor_angle_raw = Get_adc_Average(ADC_Channel_14, 15);

		// LCD: display angle data
		LCD_Show_Angle(vcc, sensor_angle_raw);

		// Serial output
		printf("ADC=%d Vbat=%.2fV\r\n", sensor_angle_raw, vcc);

		delay_ms(50);
	}
}
