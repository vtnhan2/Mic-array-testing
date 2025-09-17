/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define mic_dout_2_Pin GPIO_PIN_1
#define mic_dout_2_GPIO_Port GPIOA
#define mic_ws_0_Pin GPIO_PIN_4
#define mic_ws_0_GPIO_Port GPIOA
#define mic_clk_0_Pin GPIO_PIN_5
#define mic_clk_0_GPIO_Port GPIOA
#define mic_dout_0_Pin GPIO_PIN_7
#define mic_dout_0_GPIO_Port GPIOA
#define mic_clk_3_Pin GPIO_PIN_0
#define mic_clk_3_GPIO_Port GPIOB
#define mic_ws_3_Pin GPIO_PIN_1
#define mic_ws_3_GPIO_Port GPIOB
#define mic_clk_1_Pin GPIO_PIN_10
#define mic_clk_1_GPIO_Port GPIOB
#define mic_ws_2_Pin GPIO_PIN_12
#define mic_ws_2_GPIO_Port GPIOB
#define mic_clk_2_Pin GPIO_PIN_13
#define mic_clk_2_GPIO_Port GPIOB
#define mic_dout_1_Pin GPIO_PIN_15
#define mic_dout_1_GPIO_Port GPIOB
#define mic_dout_3_Pin GPIO_PIN_10
#define mic_dout_3_GPIO_Port GPIOA
#define mic_led_clk_Pin GPIO_PIN_3
#define mic_led_clk_GPIO_Port GPIOB
#define mic_led_data_Pin GPIO_PIN_5
#define mic_led_data_GPIO_Port GPIOB
#define mic_ws_1_Pin GPIO_PIN_9
#define mic_ws_1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
