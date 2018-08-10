/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__

/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */
#include <stdint.h>

#define BNO055_HEADING (((uint16_t)currBnoBuffer[0x1B] << 8 | currBnoBuffer[0x1A])/16.0f)
#define BNO055_ROLL ((int16_t)((uint16_t)currBnoBuffer[0x1D] << 8 | currBnoBuffer[0x1C])/16.0f)
#define BNO055_HEADING_PM (BNO055_HEADING > 180 ? BNO055_HEADING-360 : BNO055_HEADING);
#define BNO055_PITCH ((int16_t)((uint16_t)currBnoBuffer[0x1F] << 8 | currBnoBuffer[0x1E])/16.0f)

#define BNO055_GYRO_X ((int16_t)((uint16_t)currBnoBuffer[0x19] << 8 | currBnoBuffer[0x18]))
#define BNO055_GYRO_Y ((int16_t)((uint16_t)currBnoBuffer[0x17] << 8 | currBnoBuffer[0x16]))
#define BNO055_GYRO_Z ((int16_t)((uint16_t)currBnoBuffer[0x15] << 8 | currBnoBuffer[0x14]))

#define BNO055_ACC_X ((int16_t)((uint16_t)currBnoBuffer[0x29] << 8 | currBnoBuffer[0x28]))
#define BNO055_ACC_Y ((int16_t)((uint16_t)currBnoBuffer[0x2B] << 8 | currBnoBuffer[0x2A]))
#define BNO055_ACC_Z ((int16_t)((uint16_t)currBnoBuffer[0x2D] << 8 | currBnoBuffer[0x2C]))

#define BNO055_TEMP (currBnoBuffer[0x34]);
#define BNO055_CALIBSTATUS (currBnoBuffer[0x35]);
#define BNO055_OPMODE (currBnoBuffer[0x3D]&(uint8_t)0b1111)
#define BNO055_SYSERR (currBnoBuffer[0x3A])

#define MPL_HEIGHT (((uint16_t) currMplBuffer[1] << 8 | currMplBuffer[2]) + (currMplBuffer[3] >> 4) / 16.0f)
#define MPL_TEMP (currMplBuffer[4] + (currMplBuffer[5] >> 4) / 16.0f)

extern uint8_t *currBnoBuffer;
extern int16_t servoPosition[5];

enum Servos {
    VTAIL_R,
    VTAIL_L,
    AILERON_L,
    AILERON_R,
    MOTOR
};
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define MCO_Pin GPIO_PIN_0
#define MCO_GPIO_Port GPIOA
#define PPM_AILERON_R_Pin GPIO_PIN_1
#define PPM_AILERON_R_GPIO_Port GPIOA
#define VCP_TX_Pin GPIO_PIN_2
#define VCP_TX_GPIO_Port GPIOA
#define VCP_RX_Pin GPIO_PIN_3
#define VCP_RX_GPIO_Port GPIOA
#define PIN_A4_Pin GPIO_PIN_5
#define PIN_A4_GPIO_Port GPIOA
#define PPM_VTAIL_L_Pin GPIO_PIN_6
#define PPM_VTAIL_L_GPIO_Port GPIOA
#define PPM_AILERON_L_Pin GPIO_PIN_7
#define PPM_AILERON_L_GPIO_Port GPIOA
#define PPM_VTAIL_R_Pin GPIO_PIN_0
#define PPM_VTAIL_R_GPIO_Port GPIOB
#define PPM_MOTOR_Pin GPIO_PIN_1
#define PPM_MOTOR_GPIO_Port GPIOB
#define SBUS_TX_Pin GPIO_PIN_9
#define SBUS_TX_GPIO_Port GPIOA
#define SBUS_RX_Pin GPIO_PIN_10
#define SBUS_RX_GPIO_Port GPIOA
#define POWER_DST_CS_Pin GPIO_PIN_11
#define POWER_DST_CS_GPIO_Port GPIOA
#define PIN_D2_Pin GPIO_PIN_12
#define PIN_D2_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define I2C_SCL_Pin GPIO_PIN_6
#define I2C_SCL_GPIO_Port GPIOB
#define I2C_SDA_Pin GPIO_PIN_7
#define I2C_SDA_GPIO_Port GPIOB

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

/* USER CODE BEGIN Private defines */

void controller_tick();
void handle_usart();

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
