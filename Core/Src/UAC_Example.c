/**
  ******************************************************************************
  * @file    UAC_Example.c
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

/* Includes ------------------------------------------------------------------*/
#include "UAC_Example.h"

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static UAC_HandleTypeDef huac;
static UAC_AudioConfig_t audio_config;

/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Example: Initialize UAC microphone device
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_Example_Init(void)
{
    HAL_StatusTypeDef status = HAL_OK;

    /* Configure audio parameters */
    audio_config.format = UAC_FORMAT_PCM;
    audio_config.sample_rate = UAC_SAMPLE_RATE_48K;
    audio_config.channels = 1;  /* Mono microphone */
    audio_config.bits_per_sample = 16;
    audio_config.frame_size = 2; /* 16-bit = 2 bytes per sample */

    /* Initialize UAC device */
    extern USBD_HandleTypeDef hUsbDeviceFS;
    status = UAC_Init(&huac, &hUsbDeviceFS, &audio_config);
    if (status != HAL_OK) {
        return status;
    }

    /* Set audio data callback */
    status = UAC_SetAudioDataCallback(&huac, UAC_Example_AudioDataCallback);
    if (status != HAL_OK) {
        return status;
    }

    /* Start audio streaming */
    status = UAC_StartStreaming(&huac);
    if (status != HAL_OK) {
        return status;
    }

    return HAL_OK;
}

/**
  * @brief  Example: Process audio data from I2S
  * @param  i2s_handle: I2S handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_Example_ProcessI2SData(I2S_HandleTypeDef *i2s_handle)
{
    HAL_StatusTypeDef status = HAL_OK;
    uint16_t audio_data[UAC_AUDIO_BUFFER_SIZE];
    uint16_t data_length = 0;

    /* Receive audio data from I2S */
    status = HAL_I2S_Receive(i2s_handle, (uint16_t*)audio_data, UAC_AUDIO_BUFFER_SIZE, HAL_MAX_DELAY);
    if (status != HAL_OK) {
        return status;
    }

    /* Process audio data through UAC */
    data_length = UAC_AUDIO_BUFFER_SIZE;
    status = UAC_ProcessAudioData(&huac, audio_data, data_length);
    if (status != HAL_OK) {
        return status;
    }

    return HAL_OK;
}

/**
  * @brief  Example: Audio data callback function
  * @param  data: Audio data pointer
  * @param  length: Data length
  * @retval None
  */
void UAC_Example_AudioDataCallback(uint16_t *data, uint16_t length)
{
    /* This callback is called when audio data is ready to be sent via USB */
    /* You can add additional processing here if needed */
    /* For example: audio filtering, volume control, etc. */
    
    /* The data is automatically sent via USB by the UAC driver */
    /* No additional action is required in this callback */
}

/* Private functions ---------------------------------------------------------*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
