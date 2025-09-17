/**
  ******************************************************************************
  * @file    MIC_ARRAY.h
  * @author  Nhan Vo
  * @date    2025-01-14
  * @brief   Mic Array Driver Header File
  *          Based on Sipeed MicArray documentation
  ******************************************************************************
  */

#ifndef __MIC_ARRAY_H
#define __MIC_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
  * @brief  Mic Array Channel Selection
  */
typedef enum {
    MIC_ARRAY_CHANNEL_LEFT = 0,    /* Left channel */
    MIC_ARRAY_CHANNEL_RIGHT = 1    /* Right channel */
} MIC_ARRAY_Channel_t;

/**
  * @brief  Mic Array Handle Structure
  */
typedef struct {
    I2S_HandleTypeDef *hi2s;           /* I2S handle pointer */
    uint16_t *audio_buffer;            /* Audio buffer pointer */
    uint16_t buffer_size;              /* Buffer size in samples */
    uint16_t buffer_index;             /* Current buffer index */
    uint8_t is_initialized;            /* Initialization flag */
    uint8_t is_streaming;              /* Streaming flag */
    uint32_t sample_rate;              /* Sample rate in Hz */
    uint8_t channels;                  /* Number of channels */
    uint16_t bits_per_sample;          /* Bits per sample */
    /* Channel selection not available on Sipeed Mic Array */
} MIC_ARRAY_HandleTypeDef;

/**
  * @brief  Mic Array Configuration Structure
  */
typedef struct {
    uint32_t sample_rate;              /* Sample rate (8000, 16000, 22050, 44100, 48000) */
    uint8_t channels;                  /* Number of channels (1-8) */
    uint16_t bits_per_sample;          /* Bits per sample (16, 24, 32) */
    uint16_t buffer_size;              /* Buffer size in samples */
    uint8_t mic_count;                 /* Number of microphones (1-8) */
} MIC_ARRAY_Config_t;

/* Exported constants --------------------------------------------------------*/

/* Mic Array Configuration */
#define MIC_ARRAY_MAX_CHANNELS          8
#define MIC_ARRAY_DEFAULT_SAMPLE_RATE   48000
#define MIC_ARRAY_DEFAULT_CHANNELS      8
#define MIC_ARRAY_DEFAULT_BITS         16
#define MIC_ARRAY_BUFFER_SIZE          512
/* MODE_PIN not available on Sipeed Mic Array */

/* I2S Configuration for Mic Array */
#define MIC_ARRAY_I2S                   I2S2
#define MIC_ARRAY_I2S_CLK_PIN           GPIO_PIN_13
#define MIC_ARRAY_I2S_CLK_PORT          GPIOB
#define MIC_ARRAY_I2S_WS_PIN            GPIO_PIN_12
#define MIC_ARRAY_I2S_WS_PORT           GPIOB
#define MIC_ARRAY_I2S_SD_PIN            GPIO_PIN_15
#define MIC_ARRAY_I2S_SD_PORT           GPIOB

/* Mic Array Pin Definitions (based on STM32F411CEUx pinout) */
#define MIC_ARRAY_MIC_COUNT             8
#define MIC_ARRAY_MIC_1_PIN             GPIO_PIN_1
#define MIC_ARRAY_MIC_1_PORT            GPIOA
#define MIC_ARRAY_MIC_2_PIN             GPIO_PIN_15
#define MIC_ARRAY_MIC_2_PORT            GPIOB
#define MIC_ARRAY_MIC_3_PIN             GPIO_PIN_10
#define MIC_ARRAY_MIC_3_PORT            GPIOA
#define MIC_ARRAY_MIC_4_PIN             GPIO_PIN_9
#define MIC_ARRAY_MIC_4_PORT            GPIOB
#define MIC_ARRAY_MIC_5_PIN             GPIO_PIN_0
#define MIC_ARRAY_MIC_5_PORT            GPIOB
#define MIC_ARRAY_MIC_6_PIN             GPIO_PIN_1
#define MIC_ARRAY_MIC_6_PORT            GPIOB
#define MIC_ARRAY_MIC_7_PIN             GPIO_PIN_2
#define MIC_ARRAY_MIC_7_PORT            GPIOB
#define MIC_ARRAY_MIC_8_PIN             GPIO_PIN_4
#define MIC_ARRAY_MIC_8_PORT            GPIOB

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  Initialize Mic Array
  * @param  hmic: Mic Array handle pointer
  * @param  hi2s: I2S handle pointer
  * @param  config: Configuration pointer
  * @retval HAL status
  */
HAL_StatusTypeDef MIC_ARRAY_Init(MIC_ARRAY_HandleTypeDef *hmic, I2S_HandleTypeDef *hi2s, MIC_ARRAY_Config_t *config);

/**
  * @brief  Deinitialize Mic Array
  * @param  hmic: Mic Array handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef MIC_ARRAY_DeInit(MIC_ARRAY_HandleTypeDef *hmic);

/**
  * @brief  Start Mic Array streaming
  * @param  hmic: Mic Array handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef MIC_ARRAY_StartStreaming(MIC_ARRAY_HandleTypeDef *hmic);

/**
  * @brief  Stop Mic Array streaming
  * @param  hmic: Mic Array handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef MIC_ARRAY_StopStreaming(MIC_ARRAY_HandleTypeDef *hmic);

/**
  * @brief  Read audio data from Mic Array
  * @param  hmic: Mic Array handle pointer
  * @param  buffer: Audio buffer pointer
  * @param  size: Buffer size in samples
  * @retval HAL status
  */
HAL_StatusTypeDef MIC_ARRAY_ReadData(MIC_ARRAY_HandleTypeDef *hmic, uint16_t *buffer, uint16_t size);

/**
  * @brief  Process Mic Array data (convert to mono for USB)
  * @param  hmic: Mic Array handle pointer
  * @param  input_buffer: Input buffer (multi-channel)
  * @param  output_buffer: Output buffer (mono)
  * @param  input_size: Input buffer size
  * @retval HAL status
  */
HAL_StatusTypeDef MIC_ARRAY_ProcessData(MIC_ARRAY_HandleTypeDef *hmic, uint16_t *input_buffer, uint16_t *output_buffer, uint16_t input_size);

/**
  * @brief  Get Mic Array status
  * @param  hmic: Mic Array handle pointer
  * @retval Status (0: not ready, 1: ready)
  */
uint8_t MIC_ARRAY_GetStatus(MIC_ARRAY_HandleTypeDef *hmic);

/**
  * @brief  Set Mic Array gain
  * @param  hmic: Mic Array handle pointer
  * @param  gain: Gain value (0-100)
  * @retval HAL status
  */
HAL_StatusTypeDef MIC_ARRAY_SetGain(MIC_ARRAY_HandleTypeDef *hmic, uint8_t gain);

/**
  * @brief  Mic Array callback function
  * @param  hmic: Mic Array handle pointer
  * @retval None
  */
void MIC_ARRAY_Callback(MIC_ARRAY_HandleTypeDef *hmic);

/**
  * @brief  Initialize Mic Array (wrapper function for main.c)
  * @retval None
  */
void MIC_ARRAY_Init_Microphones(void);

/**
  * @brief  Process Mic Array Audio (wrapper function for main.c)
  * @retval None
  */
void MIC_ARRAY_Process_Audio(void);

/* Channel selection functions not available on Sipeed Mic Array */

#ifdef __cplusplus
}
#endif

#endif /* __MIC_ARRAY_H */
