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
#include "IOI2C.h"      // Software I2C for MPU6050
#include "MPU6050.h"    // MPU6050 driver
#include "filter.h"     // Kalman filter

 /**************************************************************************
WHEELTEC D24Ademo - MPU6050 Gyroscope Testing Mode
MOTORS DISABLED - angle sensor testing only
**************************************************************************/

float MPU_Pitch, MPU_Roll;                // MPU6050 Euler angles
float MPU_GyroX, MPU_GyroY, MPU_GyroZ;   // Gyro data
int   mpu_connected = 0;                 // MPU6050 connection status

// LCD display for MPU6050 sensor data
void LCD_Show_MPU6050(float vcc)
{
	// Row 1: Title
	LCD_ShowString(0,  0, (u8*)"MPU6050 Test   ", WHITE, BLACK, 16, 0);

	// Row 2: Pitch angle
	LCD_ShowString(0,  16, (u8*)"Pitch:", WHITE, BLACK, 16, 0);
	LCD_ShowFloatNum1(48, 16, Pitch, 5, GREEN, BLACK, 16);

	// Row 3: Roll angle
	LCD_ShowString(0,  32, (u8*)"Roll: ", WHITE, BLACK, 16, 0);
	LCD_ShowFloatNum1(48, 32, Roll, 5, GREEN, BLACK, 16);

	// Row 4: Gyro X, Y
	LCD_ShowString(0,  48, (u8*)"GX:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(24, 48, (int)MPU_GyroX, 5, YELLOW, BLACK, 16);
	LCD_ShowString(64, 48, (u8*)"GY:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(88, 48, (int)MPU_GyroY, 5, YELLOW, BLACK, 16);

	// Row 5: Gyro Z + Battery voltage
	LCD_ShowString(0,  64, (u8*)"GZ:", WHITE, BLACK, 16, 0);
	LCD_ShowIntNum(24, 64, (int)MPU_GyroZ, 5, YELLOW, BLACK, 16);
	LCD_ShowString(64, 64, (u8*)"V:", WHITE, BLACK, 16, 0);
	LCD_ShowFloatNum1(80, 64, vcc, 4, GREEN, BLACK, 16);

	// Row 6: MPU6050 connection status
	LCD_ShowString(0,  80, (u8*)"MPU:", WHITE, BLACK, 16, 0);
	if (mpu_connected) {
		LCD_ShowString(40, 80, (u8*)"Connected  ", GREEN, BLACK, 16, 0);
	} else {
		LCD_ShowString(40, 80, (u8*)"Not Found  ", RED, BLACK, 16, 0);
	}
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

	// ---- MOTORS DISABLED for MPU6050 sensor testing ----
	// PWM_Int(7199,0);
	// Encoder_Init_Tim8();
	// Encoder_Init_Tim2();
	// Encoder_Init_Tim3();
	// Encoder_Init_Soft();       // D encoder software decode PA8/PA4
	// Encoder_Timer_Init();      // 1ms timer polling D encoder

	// Initialize LCD
	LCD_Init();
	LCD_Fill(0, 0, 128, 128, BLACK);

	// Initialize I2C and MPU6050
	IIC_Init();
	delay_ms(100);
	MPU6050_initialize();
	delay_ms(50);
	if (MPU6050_testConnection()) {
		printf("MPU6050 connected!\r\n");
		DMP_Init();  // Initialize DMP for angle calculation
		printf("DMP initialized!\r\n");
		mpu_connected = 1;
	} else {
		printf("MPU6050 NOT found!\r\n");
		mpu_connected = 0;
	}

	while(1)
	{
		// Read battery voltage
		adcx = Get_adc_Average(ADC_Channel_5, 10);
		vcc = (float)adcx * (3.3 * 11 / 4096);

		// Read MPU6050 DMP data
		if (mpu_connected) {
			Read_DMP();
			MPU_Pitch = Pitch;
			MPU_Roll  = Roll;
			MPU_GyroX = gyro[0];
			MPU_GyroY = gyro[1];
			MPU_GyroZ = gyro[2];
		}

		// ---- MOTORS DISABLED - encoder / PI / PWM commented out ----
		// encoder[0] = Read_Encoder(8);
		// encoder[1] = Read_Encoder(2);
		// encoder[2] = Read_Encoder(3);
		// encoder[3] = Read_Encoder(4);
		// pwm[0] = Velocity_A(TargetVelocity, encoder[0]);
		// pwm[1] = Velocity_B(TargetVelocity, encoder[1]);
		// pwm[2] = Velocity_C(TargetVelocity, encoder[2]);
		// pwm[3] = Velocity_D(TargetVelocity, encoder[3]);
		// Set_PWM(pwm[0], pwm[1], pwm[2], pwm[3]);

		// LCD: display MPU6050 sensor data
		LCD_Show_MPU6050(vcc);

		// Serial output: MPU6050 angle data
		printf("Pitch=%.2f Roll=%.2f V=%.2f\r\n", Pitch, Roll, vcc);

		delay_ms(50);
	}
}
