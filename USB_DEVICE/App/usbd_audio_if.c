/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_audio_if.c
  * @version        : v1.0_Cube
  * @brief          : Generic media access layer.
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

/* Includes ------------------------------------------------------------------*/
#include "usbd_audio_if.h"

/* USER CODE BEGIN INCLUDE */
#include "UAC.h"
#include "audio.h"
#include <math.h>
/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
extern UART_HandleTypeDef huart2;

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device library.
  * @{
  */

/** @addtogroup USBD_AUDIO_IF
  * @{
  */

/** @defgroup USBD_AUDIO_IF_Private_TypesDefinitions USBD_AUDIO_IF_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_AUDIO_IF_Private_Defines USBD_AUDIO_IF_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */

/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_AUDIO_IF_Private_Macros USBD_AUDIO_IF_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_AUDIO_IF_Private_Variables USBD_AUDIO_IF_Private_Variables
  * @brief Private variables.
  * @{
  */

/* USER CODE BEGIN PRIVATE_VARIABLES */
extern UAC_HandleTypeDef huac;
/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_AUDIO_IF_Exported_Variables USBD_AUDIO_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_AUDIO_IF_Private_FunctionPrototypes USBD_AUDIO_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t AUDIO_Init_FS(uint32_t AudioFreq, uint32_t Volume, uint32_t options);
static int8_t AUDIO_DeInit_FS(uint32_t options);
static int8_t AUDIO_AudioCmd_FS(uint8_t* pbuf, uint32_t size, uint8_t cmd);
static int8_t AUDIO_VolumeCtl_FS(uint8_t vol);
static int8_t AUDIO_MuteCtl_FS(uint8_t cmd);
static int8_t AUDIO_PeriodicTC_FS(uint8_t *pbuf, uint32_t size, uint8_t cmd);
static int8_t AUDIO_GetState_FS(void);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_AUDIO_ItfTypeDef USBD_AUDIO_fops_FS =
{
  AUDIO_Init_FS,
  AUDIO_DeInit_FS,
  AUDIO_AudioCmd_FS,
  AUDIO_VolumeCtl_FS,
  AUDIO_MuteCtl_FS,
  AUDIO_PeriodicTC_FS,
  AUDIO_GetState_FS,
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the AUDIO media low layer over USB FS IP
  * @param  AudioFreq: Audio frequency used to play the audio stream.
  * @param  Volume: Initial volume level (from 0 (Mute) to 100 (Max))
  * @param  options: Reserved for future use
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_Init_FS(uint32_t AudioFreq, uint32_t Volume, uint32_t options)
{
  /* USER CODE BEGIN 0 */
  UNUSED(AudioFreq);
  UNUSED(Volume);
  UNUSED(options);
  
  /* Mark UAC as configured */
  huac.is_configured = 1;
  
  return (USBD_OK);
  /* USER CODE END 0 */
}

/**
  * @brief  De-Initializes the AUDIO media low layer
  * @param  options: Reserved for future use
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_DeInit_FS(uint32_t options)
{
  /* USER CODE BEGIN 1 */
  UNUSED(options);
  
  /* Mark UAC as not configured */
  huac.is_configured = 0;
  huac.is_streaming = 0;
  
  return (USBD_OK);
  /* USER CODE END 1 */
}

/**
  * @brief  Handles AUDIO command.
  * @param  pbuf: Pointer to buffer of data to be sent
  * @param  size: Number of data to be sent (in bytes)
  * @param  cmd: Command opcode
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_AudioCmd_FS(uint8_t* pbuf, uint32_t size, uint8_t cmd)
{
  /* USER CODE BEGIN 2 */
  // Debug: Print every AudioCmd call
  uint8_t debug_msg[100];
  int len = snprintf((char*)debug_msg, sizeof(debug_msg), 
      "[USB_AUDIO] AudioCmd called: cmd=%d, size=%lu\r\n", cmd, size);
  HAL_UART_Transmit(&huart2, debug_msg, len, 1000);
  
  switch(cmd)
  {
    case AUDIO_CMD_START:
      /* Start audio streaming - for microphone input */
      uint8_t start_msg[] = "[USB_AUDIO] AUDIO_CMD_START received\r\n";
      HAL_UART_Transmit(&huart2, start_msg, sizeof(start_msg)-1, 1000);
      huac.is_streaming = 1;
      Audio_USB_Start_Streaming();
      break;

    case AUDIO_CMD_PLAY:
      /* For microphone, this is actually recording start */
      uint8_t play_msg[] = "[USB_AUDIO] AUDIO_CMD_PLAY received\r\n";
      HAL_UART_Transmit(&huart2, play_msg, sizeof(play_msg)-1, 1000);
      huac.is_streaming = 1;
      Audio_USB_Start_Streaming();
      break;
      
    case AUDIO_CMD_STOP:
      /* Stop audio streaming */
      uint8_t stop_msg[] = "[USB_AUDIO] AUDIO_CMD_STOP received\r\n";
      HAL_UART_Transmit(&huart2, stop_msg, sizeof(stop_msg)-1, 1000);
      huac.is_streaming = 0;
      Audio_USB_Stop_Streaming();
      break;
      
    default:
      uint8_t unknown_msg[] = "[USB_AUDIO] Unknown command received\r\n";
      HAL_UART_Transmit(&huart2, unknown_msg, sizeof(unknown_msg)-1, 1000);
      break;
  }
  UNUSED(pbuf);
  UNUSED(size);
  return (USBD_OK);
  /* USER CODE END 2 */
}

/**
  * @brief  Controls AUDIO Volume.
  * @param  vol: volume level (0..100)
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_VolumeCtl_FS(uint8_t vol)
{
  /* USER CODE BEGIN 3 */
  printf("[USB_AUDIO] VolumeCtl called: vol=%d\r\n", vol);
  // Set microphone volume (0-100)
  Audio_Set_Volume((uint16_t)vol);
  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  Controls AUDIO Mute.
  * @param  cmd: command opcode
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_MuteCtl_FS(uint8_t cmd)
{
  /* USER CODE BEGIN 4 */
  printf("[USB_AUDIO] MuteCtl called: cmd=%d\r\n", cmd);
  // Control microphone mute
  if (cmd == 1) {
    Audio_Mute();
  } else {
    Audio_UnMute();
  }
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  AUDIO_PeriodicT_FS
  * @param  cmd: Command opcode
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_PeriodicTC_FS(uint8_t *pbuf, uint32_t size, uint8_t cmd)
{
  /* USER CODE BEGIN 5 */
  static uint32_t periodic_call_count = 0;
  periodic_call_count++;
  
  // Debug: Print first few calls
  if (periodic_call_count <= 10) {
    printf("[USB_AUDIO] PeriodicTC called: count=%lu, cmd=%d, size=%lu\r\n", 
           periodic_call_count, cmd, size);
  }
  
  // Debug: Print every 100 calls to track activity
  if (periodic_call_count % 100 == 0) {
    printf("[USB_AUDIO] PeriodicTC: count=%lu, cmd=%d, size=%lu\r\n", 
           periodic_call_count, cmd, size);
  }
  
  if (cmd == AUDIO_IN_TC) {
    // Debug: Track periodic calls for microphone
    if (periodic_call_count % 1000 == 0) {
      printf("USB PeriodicTC IN: %lu, size=%lu\r\n", periodic_call_count, size);
    }
    
    // Microphone data transmission
    if (pbuf != NULL && size > 0) {
      printf("[USB_AUDIO] Getting audio data: size=%lu\r\n", size);
      
      // Get audio data from I2S processing and send to USB
      uint16_t bytes_filled = Audio_USB_Get_Next_Packet(pbuf, size);
      
      printf("[USB_AUDIO] Audio data: %d bytes filled\r\n", bytes_filled);

      // Return the actual number of bytes filled
      return (bytes_filled > 0) ? USBD_OK : USBD_FAIL;
    }

    // Fill with silence if not ready
    if (pbuf != NULL && size > 0) {
      printf("[USB_AUDIO] Filling with silence: size=%lu\r\n", size);
      memset(pbuf, 0, size);
    }
  }
  else {
    // Speaker data reception (legacy)
    printf("[USB_AUDIO] Speaker data reception (legacy)\r\n");
    UNUSED(pbuf);
    UNUSED(size);
  }

  return (USBD_OK);
  /* USER CODE END 5 */
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */
void AUDIO_Start_Microphone_Transmission(void)
{
  // Start microphone data transmission to USB host
  extern USBD_HandleTypeDef hUsbDeviceFS;
  
  uint8_t start_msg[] = "[USB_AUDIO] ===== AUDIO_Start_Microphone_Transmission called =====\r\n";
  HAL_UART_Transmit(&huart2, start_msg, sizeof(start_msg)-1, 1000);
  
  uint8_t streaming_msg[50];
  int len = snprintf((char*)streaming_msg, sizeof(streaming_msg), 
      "[USB_AUDIO] huac.is_streaming: %d\r\n", huac.is_streaming);
  HAL_UART_Transmit(&huart2, streaming_msg, len, 1000);
  
  if (huac.is_streaming) {
    // Start USB audio streaming first
    Audio_USB_Start_Streaming();
    
    uint8_t streaming_started_msg[] = "[USB_AUDIO] Audio_USB_Start_Streaming called\r\n";
    HAL_UART_Transmit(&huart2, streaming_started_msg, sizeof(streaming_started_msg)-1, 1000);
    
    // Force enable IN endpoint and start streaming
    uint8_t endpoint_msg[] = "Starting USB IN endpoint transmission...\r\n";
    HAL_UART_Transmit(&huart2, endpoint_msg, sizeof(endpoint_msg)-1, 1000);
    
    uint8_t trigger_msg[] = "[USB_AUDIO] Triggering initial transmission...\r\n";
    HAL_UART_Transmit(&huart2, trigger_msg, sizeof(trigger_msg)-1, 1000);
    
    // Trigger initial transmission to kickstart the endpoint
    uint8_t initial_buffer[96]; // USB_AUDIO_PACKET_SIZE * 2
    memset(initial_buffer, 0, sizeof(initial_buffer)); // Start with silence
    
    // Start transmission on IN endpoint
    HAL_StatusTypeDef status = USBD_LL_Transmit(&hUsbDeviceFS, 0x81, initial_buffer, sizeof(initial_buffer));
    
    uint8_t status_msg[50];
    len = snprintf((char*)status_msg, sizeof(status_msg), 
        "[USB_AUDIO] USBD_LL_Transmit status: %d\r\n", status);
    HAL_UART_Transmit(&huart2, status_msg, len, 1000);
    
    // Also try to trigger any pending transfers
    extern USBD_AUDIO_ItfTypeDef USBD_AUDIO_fops_FS;
    int8_t result = USBD_AUDIO_fops_FS.PeriodicTC(initial_buffer, sizeof(initial_buffer), AUDIO_IN_TC);
    
    uint8_t result_msg[50];
    len = snprintf((char*)result_msg, sizeof(result_msg), 
        "[USB_AUDIO] PeriodicTC result: %d\r\n", result);
    HAL_UART_Transmit(&huart2, result_msg, len, 1000);
    
    uint8_t done_msg[] = "[USB_AUDIO] ===== Microphone transmission started =====\r\n";
    HAL_UART_Transmit(&huart2, done_msg, sizeof(done_msg)-1, 1000);
  } else {
    uint8_t error_msg[] = "[USB_AUDIO] ERROR: huac.is_streaming is FALSE - cannot start transmission\r\n";
    HAL_UART_Transmit(&huart2, error_msg, sizeof(error_msg)-1, 1000);
  }
}
/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @brief  Gets AUDIO State.
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_GetState_FS(void)
{
  /* USER CODE BEGIN 6 */
  return (USBD_OK);
  /* USER CODE END 6 */
}

/**
  * @brief  Manages the DMA full transfer complete event.
  * @retval None
  */
void TransferComplete_CallBack_FS(void)
{
  /* USER CODE BEGIN 7 */
  USBD_AUDIO_Sync(&hUsbDeviceFS, AUDIO_OFFSET_FULL);
  /* USER CODE END 7 */
}

/**
  * @brief  Manages the DMA Half transfer complete event.
  * @retval None
  */
void HalfTransfer_CallBack_FS(void)
{
  /* USER CODE BEGIN 8 */
  USBD_AUDIO_Sync(&hUsbDeviceFS, AUDIO_OFFSET_HALF);
  /* USER CODE END 8 */
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */
