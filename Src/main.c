/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32l4xx_hal.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include "../rc_lib/rc_lib.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
#define HEADING (((uint16_t)currBnoBuffer[0x1B] << 8 | currBnoBuffer[0x1A])/16.0)
#define ROLL (((uint16_t)currBnoBuffer[0x1D] << 8 | currBnoBuffer[0x1C])/16.0)
#define PITCH (((uint16_t)currBnoBuffer[0x1F] << 8 | currBnoBuffer[0x1E])/16.0)


uint8_t bnoBuffer0[64], bnoBuffer1[64];
uint8_t *currBnoBuffer;
float height = 0;
float temp = 2;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

bool handle_i2c() {
    static uint8_t state = 0;

    uint8_t reg;
    uint8_t buf[5];

    switch (state++) {
        case 0:
            reg = 0;
            HAL_I2C_Master_Transmit_DMA(&hi2c1, 0x28 << 1, &reg, sizeof(reg));
            break;
        case 1:
            if(currBnoBuffer == bnoBuffer0) {
                currBnoBuffer = bnoBuffer1;
                HAL_I2C_Master_Receive_DMA(&hi2c1, 0x28 << 1, bnoBuffer0, sizeof(bnoBuffer0));
            } else {
                currBnoBuffer = bnoBuffer0;
                HAL_I2C_Master_Receive_DMA(&hi2c1, 0x28 << 1, bnoBuffer1, sizeof(bnoBuffer1));
            }
            break;
        case 2:
            reg = 1;
            HAL_I2C_Master_Transmit_DMA(&hi2c1, 0xC0, &reg, sizeof(reg));
            break;
        case 3:
            HAL_I2C_Master_Receive_DMA(&hi2c1, 0xC0, buf, sizeof(buf));
            if(buf[0] != 0 || buf[1] != 0 || buf[2] != 0) {
                height = ((uint16_t) buf[1] << 8 | buf[2]) + (buf[0] >> 4) / 16.0f;
            }
            if(buf[3] != 0 || buf[4] != 0) {
                temp = buf[4] + (buf[3]>>4)/16.0f;
            }
            break;
        default:
            state = 0;
            return true;
    }
    return false;
}

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_TIM1_Init();
  MX_TIM16_Init();
  MX_USART2_UART_Init();

  /* USER CODE BEGIN 2 */
    HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);

    // Timer starten
    HAL_TIM_Base_Start(&htim1);
    HAL_TIM_Base_Start(&htim2);
    HAL_TIM_Base_Start(&htim16);

    HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim16,TIM_CHANNEL_1);

    //BNO-055 Konfigurieren
    uint8_t cmd[2];
    cmd[0] = 0x3B; //UNIT_SEL
    cmd[1] = 0 << 0 | 1 << 1 | 0 << 2
             | 0 << 4 | 0 << 7;

    HAL_I2C_Master_Transmit_DMA(&hi2c1, 0x28  << 1, cmd, sizeof(cmd));

    cmd[0] = 0x3D;  //OPR_MODE
    cmd[1] = 0b1011;

    HAL_I2C_Master_Transmit_DMA(&hi2c1, 0x28  << 1, cmd, sizeof(cmd));

    currBnoBuffer = bnoBuffer0;

    HAL_Delay(20);

    // MPL konfigurieren
    cmd[0] = 0x26; // CTRL-Reg
    cmd[1] = (0b1 << 7 | 0b0 << 3); // Oversampling x1/Altimeter
    HAL_I2C_Master_Transmit_DMA(&hi2c1, 0xC0, cmd, sizeof(cmd));

    // Enable Data Ready Flags
    cmd[0] = 0x13;
    cmd[1] = 0b111;
    HAL_I2C_Master_Transmit_DMA(&hi2c1, 0xC0, cmd, sizeof(cmd));

    // Set active
    cmd[0] = 0x26;
    cmd[1] = 0xB9;
    HAL_I2C_Master_Transmit_DMA(&hi2c1, 0xC0, cmd, sizeof(cmd));
    HAL_Delay(1000);
    HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

    rc_lib_package_t package;
    package.mesh = false;
    package.channel_count = 8;
    package.resolution = 1024;

    rc_lib_transmitter_id = 23;

    while (1) {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
        if(hi2c1.State == HAL_I2C_STATE_READY) {
            if(handle_i2c()) {
                package.channel_data[0] = (uint16_t)HEADING;
                package.channel_data[1] = (uint16_t)ROLL;
                package.channel_data[2] = (uint16_t)PITCH;
                package.channel_data[3] = (uint16_t)height;
                package.channel_data[4] = (uint16_t)temp;

                uint16_t length = rc_lib_encode(&package);
                HAL_UART_Transmit_DMA(&huart2, package.buffer, length);
            }
        }
    }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Configure LSE Drive Capability 
    */
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the main internal regulator output voltage 
    */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    /**Enable MSI Auto calibration 
    */
  HAL_RCCEx_EnableMSIPLLMode();

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    while (1) {
    }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
