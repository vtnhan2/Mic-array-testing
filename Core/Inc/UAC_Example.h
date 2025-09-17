/**
  ******************************************************************************
  * @file    UAC_Example.h
  * @brief   USB Audio Class (UAC) Driver Usage Example
  * @author  Nhan Vo
  * @version 1.0
  * @date    2025-09-14
  ******************************************************************************
  * @attention
  *
  * This file provides an example of how to use the UAC driver to create
  * a USB microphone device with STM32 microcontrollers.
  *
  ******************************************************************************
  */

#ifndef __UAC_EXAMPLE_H
#define __UAC_EXAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "UAC.h"
#include "main.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macros -----------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  Example: Initialize UAC microphone device
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_Example_Init(void);

/**
  * @brief  Example: Process audio data from I2S
  * @param  i2s_handle: I2S handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_Example_ProcessI2SData(I2S_HandleTypeDef *i2s_handle);

/**
  * @brief  Example: Audio data callback function
  * @param  data: Audio data pointer
  * @param  length: Data length
  * @retval None
  */
void UAC_Example_AudioDataCallback(uint16_t *data, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* __UAC_EXAMPLE_H */
