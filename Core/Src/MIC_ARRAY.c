/**
  ******************************************************************************
  * @file    MIC_ARRAY.c
  * @author  Nhan Vo
  * @date    2025-01-14
  * @brief   Mic Array Driver Implementation
  *          Based on Sipeed MicArray documentation
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "MIC_ARRAY.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static uint16_t mic_array_buffer[MIC_ARRAY_BUFFER_SIZE * MIC_ARRAY_MAX_CHANNELS];
static uint8_t mic_array_gain = 50;  /* Default gain 50% */

/* Global variables for I2S DMA callbacks */
static MIC_ARRAY_HandleTypeDef *g_hmic_array = NULL;
static volatile uint8_t dma_half_complete = 0;
static volatile uint8_t dma_full_complete = 0;

/* Private function prototypes -----------------------------------------------*/
static void MIC_ARRAY_GPIO_Init(void);
static void MIC_ARRAY_I2S_Init(I2S_HandleTypeDef *hi2s, uint32_t sample_rate);
static HAL_StatusTypeDef MIC_ARRAY_ConvertToMono(uint16_t *input, uint16_t *output, uint16_t size, uint8_t channels);
static void MIC_ARRAY_ApplyGain(uint16_t *buffer, uint16_t size, uint8_t gain);

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Initialize Mic Array
  * @param  hmic: Mic Array handle pointer
  * @param  hi2s: I2S handle pointer
  * @param  config: Configuration pointer
  * @retval HAL status
  */
HAL_StatusTypeDef MIC_ARRAY_Init(MIC_ARRAY_HandleTypeDef *hmic, I2S_HandleTypeDef *hi2s, MIC_ARRAY_Config_t *config)
{
    printf("[MIC_ARRAY] MIC_ARRAY_Init called\r\n");
    printf("[MIC_ARRAY] Parameters: hmic=%p, hi2s=%p, config=%p\r\n", hmic, hi2s, config);
    
    if (hmic == NULL || hi2s == NULL || config == NULL) {
        printf("[MIC_ARRAY] Error: NULL parameters\r\n");
        return HAL_ERROR;
    }

    /* Initialize handle */
    hmic->hi2s = hi2s;
    hmic->audio_buffer = mic_array_buffer;
    hmic->buffer_size = config->buffer_size;
    hmic->buffer_index = 0;
    hmic->is_initialized = 0;
    hmic->is_streaming = 0;
    hmic->sample_rate = config->sample_rate;
    hmic->channels = config->channels;
    hmic->bits_per_sample = config->bits_per_sample;
    /* Channel selection not available on Sipeed Mic Array */

    /* Initialize GPIO pins for mic array */
    printf("[MIC_ARRAY] Initializing GPIO...\r\n");
    MIC_ARRAY_GPIO_Init();

    /* Initialize I2S for mic array */
    printf("[MIC_ARRAY] Initializing I2S...\r\n");
    MIC_ARRAY_I2S_Init(hi2s, config->sample_rate);

    /* Clear audio buffer */
    printf("[MIC_ARRAY] Clearing audio buffer...\r\n");
    memset(hmic->audio_buffer, 0, hmic->buffer_size * sizeof(uint16_t));

    /* Set global handle for DMA callbacks */
    g_hmic_array = hmic;
    
    /* Mark as initialized */
    hmic->is_initialized = 1;
    printf("[MIC_ARRAY] Initialization completed successfully!\r\n");

    return HAL_OK;
}

/**
  * @brief  Deinitialize Mic Array
  * @param  hmic: Mic Array handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef MIC_ARRAY_DeInit(MIC_ARRAY_HandleTypeDef *hmic)
{
    if (hmic == NULL) {
        return HAL_ERROR;
    }

    /* Stop streaming if active */
    if (hmic->is_streaming) {
        MIC_ARRAY_StopStreaming(hmic);
    }

    /* Deinitialize I2S */
    if (hmic->hi2s != NULL) {
        HAL_I2S_DeInit(hmic->hi2s);
    }

    /* Clear handle */
    memset(hmic, 0, sizeof(MIC_ARRAY_HandleTypeDef));

    return HAL_OK;
}

/**
  * @brief  Start Mic Array streaming
  * @param  hmic: Mic Array handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef MIC_ARRAY_StartStreaming(MIC_ARRAY_HandleTypeDef *hmic)
{
    if (hmic == NULL || !hmic->is_initialized) {
        printf("[MIC_ARRAY] StartStreaming failed: handle=%p, init=%d\r\n", hmic, hmic ? hmic->is_initialized : 0);
        return HAL_ERROR;
    }

    printf("[MIC_ARRAY] Starting I2S DMA reception...\r\n");
    printf("[MIC_ARRAY] Buffer size: %d, Channels: %d\r\n", hmic->buffer_size, hmic->channels);
    
    /* Start I2S reception with circular DMA */
    /* For 4-channel I2S, we need to receive 4 samples per frame */
    uint16_t samples_per_frame = hmic->channels;  /* 4 channels */
    uint16_t total_samples = hmic->buffer_size * samples_per_frame;
    
    printf("[MIC_ARRAY] Starting circular DMA: %d samples per frame, %d total samples\r\n", 
           samples_per_frame, total_samples);
    
    HAL_StatusTypeDef status = HAL_I2S_Receive_DMA(hmic->hi2s, (uint16_t*)hmic->audio_buffer, total_samples);
    
    if (status == HAL_OK) {
        hmic->is_streaming = 1;
        printf("[MIC_ARRAY] I2S DMA reception started successfully!\r\n");
    } else {
        printf("[MIC_ARRAY] I2S DMA reception failed: %d\r\n", status);
    }

    return status;
}

/**
  * @brief  Stop Mic Array streaming
  * @param  hmic: Mic Array handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef MIC_ARRAY_StopStreaming(MIC_ARRAY_HandleTypeDef *hmic)
{
    if (hmic == NULL) {
        return HAL_ERROR;
    }

    /* Stop I2S reception */
    HAL_I2S_DMAStop(hmic->hi2s);
    
    hmic->is_streaming = 0;
    hmic->buffer_index = 0;

    return HAL_OK;
}

/**
  * @brief  Read audio data from Mic Array
  * @param  hmic: Mic Array handle pointer
  * @param  buffer: Audio buffer pointer
  * @param  size: Buffer size in samples
  * @retval HAL status
  */
HAL_StatusTypeDef MIC_ARRAY_ReadData(MIC_ARRAY_HandleTypeDef *hmic, uint16_t *buffer, uint16_t size)
{
    if (hmic == NULL || buffer == NULL || size == 0) {
        return HAL_ERROR;
    }

    if (!hmic->is_streaming) {
        return HAL_ERROR;
    }

    /* Check if we have new DMA data */
    if (dma_half_complete || dma_full_complete) {
        /* For circular DMA, we can read from the buffer directly */
        uint16_t copy_size = (size < hmic->buffer_size) ? size : hmic->buffer_size;
        
        /* Copy the latest data from the circular buffer */
        memcpy(buffer, (void*)hmic->audio_buffer, copy_size * sizeof(uint16_t));
        
        /* Clear DMA flags */
        dma_half_complete = 0;
        dma_full_complete = 0;
        
        printf("[MIC_ARRAY] Read %d samples from DMA buffer\r\n", copy_size);
        return HAL_OK;
    }
    
    /* No new data available */
    return HAL_BUSY;
}

/**
  * @brief  Process Mic Array data (convert to mono for USB)
  * @param  hmic: Mic Array handle pointer
  * @param  input_buffer: Input buffer (multi-channel)
  * @param  output_buffer: Output buffer (mono)
  * @param  input_size: Input buffer size
  * @retval HAL status
  */
HAL_StatusTypeDef MIC_ARRAY_ProcessData(MIC_ARRAY_HandleTypeDef *hmic, uint16_t *input_buffer, uint16_t *output_buffer, uint16_t input_size)
{
    if (hmic == NULL || input_buffer == NULL || output_buffer == NULL || input_size == 0) {
        return HAL_ERROR;
    }

    /* Convert multi-channel to mono */
    HAL_StatusTypeDef status = MIC_ARRAY_ConvertToMono(input_buffer, output_buffer, input_size, hmic->channels);
    
    if (status == HAL_OK) {
        /* Apply gain */
        MIC_ARRAY_ApplyGain(output_buffer, input_size / hmic->channels, mic_array_gain);
    }

    return status;
}

/**
  * @brief  Get Mic Array status
  * @param  hmic: Mic Array handle pointer
  * @retval Status (0: not ready, 1: ready)
  */
uint8_t MIC_ARRAY_GetStatus(MIC_ARRAY_HandleTypeDef *hmic)
{
    if (hmic == NULL) {
        return 0;
    }

    return (hmic->is_initialized && hmic->is_streaming) ? 1 : 0;
}

/**
  * @brief  Set Mic Array gain
  * @param  hmic: Mic Array handle pointer
  * @param  gain: Gain value (0-100)
  * @retval HAL status
  */
HAL_StatusTypeDef MIC_ARRAY_SetGain(MIC_ARRAY_HandleTypeDef *hmic, uint8_t gain)
{
    if (hmic == NULL) {
        return HAL_ERROR;
    }

    if (gain > 100) {
        gain = 100;
    }

    mic_array_gain = gain;
    return HAL_OK;
}

/**
  * @brief  Mic Array callback function
  * @param  hmic: Mic Array handle pointer
  * @retval None
  */
void MIC_ARRAY_Callback(MIC_ARRAY_HandleTypeDef *hmic)
{
    if (hmic == NULL) {
        return;
    }

    /* Update buffer index for next transfer */
    hmic->buffer_index = (hmic->buffer_index + 1) % hmic->buffer_size;
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initialize GPIO pins for mic array
  * @retval None
  */
static void MIC_ARRAY_GPIO_Init(void)
{
    /* I2S2 pins are already configured in main.c */
    /* This function is kept for compatibility but does nothing */
    /* MODE_PIN is configured separately in MIC_ARRAY_InitModePin() */
}

/**
  * @brief  Initialize I2S for mic array
  * @param  hi2s: I2S handle pointer
  * @param  sample_rate: Sample rate in Hz
  * @retval None
  */
static void MIC_ARRAY_I2S_Init(I2S_HandleTypeDef *hi2s, uint32_t sample_rate)
{
    /* I2S2 is already configured in main.c */
    /* This function is kept for compatibility but does nothing */
    (void)hi2s;
    (void)sample_rate;
}

/**
  * @brief  Convert multi-channel audio to mono
  * @param  input: Input buffer (multi-channel)
  * @param  output: Output buffer (mono)
  * @param  size: Input buffer size
  * @param  channels: Number of input channels
  * @retval HAL status
  */
static HAL_StatusTypeDef MIC_ARRAY_ConvertToMono(uint16_t *input, uint16_t *output, uint16_t size, uint8_t channels)
{
    if (input == NULL || output == NULL || size == 0 || channels == 0) {
        return HAL_ERROR;
    }

    uint16_t output_size = size / channels;
    
    printf("[MIC_ARRAY] ConvertToMono: input_size=%d, channels=%d, output_size=%d\r\n", 
           size, channels, output_size);
    
    for (uint16_t i = 0; i < output_size; i++) {
        int32_t sum = 0;
        
        /* Sum all channels for this sample */
        for (uint8_t ch = 0; ch < channels; ch++) {
            uint16_t sample = input[i * channels + ch];
            /* Convert from unsigned 16-bit to signed for processing */
            int16_t signed_sample = (int16_t)(sample - 32768);
            sum += signed_sample;
        }
        
        /* Average and convert back to unsigned 16-bit */
        int16_t avg_sample = (int16_t)(sum / channels);
        output[i] = (uint16_t)(avg_sample + 32768);
        
        /* Debug first few samples */
        if (i < 4) {
            printf("[MIC_ARRAY] Sample %d: input=[%d,%d,%d,%d] -> output=%d\r\n", 
                   i, input[i*channels], input[i*channels+1], input[i*channels+2], input[i*channels+3], output[i]);
        }
    }

    return HAL_OK;
}

/**
  * @brief  Apply gain to audio buffer
  * @param  buffer: Audio buffer pointer
  * @param  size: Buffer size
  * @param  gain: Gain value (0-100)
  * @retval None
  */
static void MIC_ARRAY_ApplyGain(uint16_t *buffer, uint16_t size, uint8_t gain)
{
    if (buffer == NULL || size == 0) {
        return;
    }

    float gain_factor = (float)gain / 100.0f;
    
    for (uint16_t i = 0; i < size; i++) {
        int32_t sample = (int32_t)buffer[i];
        sample = (int32_t)(sample * gain_factor);
        
        /* Clamp to 16-bit range */
        if (sample > 65535) sample = 65535;
        if (sample < 0) sample = 0;
        
        buffer[i] = (uint16_t)sample;
    }
}

/* I2S Callback Functions ---------------------------------------------------*/

/* Old callback functions removed - using new ones below */

/* Additional functions for main.c integration ---------------------------------*/

/* Channel selection functions not available on Sipeed Mic Array */

/**
  * @brief  I2S DMA Half Complete Callback
  * @param  hi2s: I2S handle pointer
  * @retval None
  */
void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (g_hmic_array != NULL && hi2s == g_hmic_array->hi2s) {
        dma_half_complete = 1;
        printf("[MIC_ARRAY] DMA Half Complete\r\n");
    }
}

/**
  * @brief  I2S DMA Full Complete Callback
  * @param  hi2s: I2S handle pointer
  * @retval None
  */
void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (g_hmic_array != NULL && hi2s == g_hmic_array->hi2s) {
        dma_full_complete = 1;
        printf("[MIC_ARRAY] DMA Full Complete\r\n");
    }
}

/**
  * @brief  Initialize Mic Array (wrapper function for main.c)
  * @retval None
  */
void MIC_ARRAY_Init_Microphones(void)
{
    /* This function should be implemented in main.c */
    /* It's declared here to avoid undefined reference errors */
}

/**
  * @brief  Process Mic Array Audio (wrapper function for main.c)
  * @retval None
  */
void MIC_ARRAY_Process_Audio(void)
{
    /* This function should be implemented in main.c */
    /* It's declared here to avoid undefined reference errors */
}
