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
float sensor_angle;         // Filtered angle (degrees)
float adc_zero_offset = 2048; // ADC zero-point offset (calibrate: sensor centered = ~mid ADC range)

// LCD显示缓冲
void LCD_Show_Angle(float vcc, int adc_raw, float angle)
{
    // Row 1: Title
    LCD_ShowString(0,  0, (u8*)"WDD35D4 Test", WHITE, BLACK, 16, 0);

    // Row 2: Raw ADC
    LCD_ShowString(0,  16, (u8*)"ADC:", WHITE, BLACK, 16, 0);
    LCD_ShowIntNum(32, 16, adc_raw, 5, YELLOW, BLACK, 16);

    // Row 3: Angle
    LCD_ShowString(0,  32, (u8*)"Angle:", WHITE, BLACK, 16, 0);
    LCD_ShowFloatNum1(48, 32, angle, 5, GREEN, BLACK, 16);
    LCD_ShowString(88, 32, (u8*)"deg", WHITE, BLACK, 16, 0);

    // Row 4: Battery voltage
    LCD_ShowString(0,  48, (u8*)"Vbat:", WHITE, BLACK, 16, 0);
    LCD_ShowFloatNum1(48, 48, vcc, 4, GREEN, BLACK, 16);
    LCD_ShowString(80, 48, (u8*)"V", WHITE, BLACK, 16, 0);

    // Row 5: Status
    LCD_ShowString(0,  64, (u8*)"Mode:Sensor Test", WHITE, BLACK, 16, 0);

    // Row 6: Motor status
    LCD_ShowString(0,  80, (u8*)"Motors: OFF", RED, BLACK, 16, 0);
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

		// Convert ADC to angle (0-4095 maps to ~0-340 degrees for WDD35D4)
		sensor_angle = (float)(sensor_angle_raw - adc_zero_offset) * 340.0 / 4096.0;

		// LCD: display angle data
		LCD_Show_Angle(vcc, sensor_angle_raw, sensor_angle);

		// Serial output
		printf("ADC=%d Angle=%.1fdeg Vbat=%.2fV\r\n", sensor_angle_raw, sensor_angle, vcc);

		delay_ms(50);
	}
}
