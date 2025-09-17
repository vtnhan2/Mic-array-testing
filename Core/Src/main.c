/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "UAC.h"
#include "MIC_ARRAY.h"
#include "usbd_audio_if.h"
#include <math.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2S_HandleTypeDef hi2s1;
I2S_HandleTypeDef hi2s2;
I2S_HandleTypeDef hi2s4;
I2S_HandleTypeDef hi2s5;
DMA_HandleTypeDef hdma_spi1_rx;
DMA_HandleTypeDef hdma_spi2_rx;
DMA_HandleTypeDef hdma_spi4_rx;
DMA_HandleTypeDef hdma_spi5_rx;

SPI_HandleTypeDef hspi3;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/* Mic Array Variables */
MIC_ARRAY_HandleTypeDef hmic_array;
MIC_ARRAY_Config_t mic_array_config;
uint8_t mic_array_initialized = 0;

/* UAC Variables */
UAC_HandleTypeDef huac;
UAC_AudioConfig_t audio_config;
uint8_t uac_initialized = 0;
uint8_t test_mode = 1;  /* 0=silence, 1=sine, 2=square, 3=noise */

/* Audio Processing Variables */
uint16_t mic_audio_buffer[MIC_ARRAY_BUFFER_SIZE];
uint16_t usb_audio_buffer[UAC_AUDIO_BUFFER_SIZE / 2];
  uint8_t use_mic_array = 1;  /* 1=use mic array, 0=use test sound */
  /* USER CODE END PV */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2S1_Init(void);
static void MX_I2S2_Init(void);
static void MX_I2S4_Init(void);
static void MX_I2S5_Init(void);
static void MX_SPI3_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
void UAC_Init_Microphone(void);
void UAC_Test_Audio(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief  Redirect printf to UART2
  * @param  ch: Character to send
  * @retval Character sent
  */
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

/**
  * @brief  Redirect getchar from UART2
  * @retval Character received
  */
int __io_getchar(void)
{
    uint8_t ch;
    HAL_UART_Receive(&huart2, &ch, 1, HAL_MAX_DELAY);
    return ch;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2S1_Init();
  MX_I2S2_Init();
  MX_I2S4_Init();
  MX_I2S5_Init();
  MX_SPI3_Init();
  MX_USART2_UART_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
  /* Initialize UAC Microphone */
  UAC_Init_Microphone();
  
  /* Initialize Mic Array */
  MIC_ARRAY_Init_Microphones();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /* Process Audio Data */
    if (uac_initialized) {
      static uint32_t debug_timer = 0;
      
      if (use_mic_array && mic_array_initialized) {
        // Use Mic Array data
        MIC_ARRAY_Process_Audio();
      } else {
        // Use test sound sequence
        UAC_TestSoundSequence(&huac);
      }
      
      // Send audio data via USB
      UAC_ProcessAudioData(&huac, huac.audio_buffer, UAC_AUDIO_BUFFER_SIZE);
      
      // Debug output every 5 seconds
      if (HAL_GetTick() - debug_timer > 5000) {
        printf("Audio Status: UAC(conf=%d,str=%d), MicArray(init=%d,str=%d), use_mic=%d\r\n", 
               huac.is_configured, huac.is_streaming, 
               mic_array_initialized, MIC_ARRAY_GetStatus(&hmic_array),
               use_mic_array);
        debug_timer = HAL_GetTick();
      }
    }
    
    /* Small delay to prevent overwhelming the system */
    HAL_Delay(1);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 192;
  PeriphClkInitStruct.PLLI2S.PLLI2SM = 16;
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2S1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2S1_Init(void)
{

  /* USER CODE BEGIN I2S1_Init 0 */

  /* USER CODE END I2S1_Init 0 */

  /* USER CODE BEGIN I2S1_Init 1 */

  /* USER CODE END I2S1_Init 1 */
  hi2s1.Instance = SPI1;
  hi2s1.Init.Mode = I2S_MODE_SLAVE_RX;
  hi2s1.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s1.Init.DataFormat = I2S_DATAFORMAT_32B;
  hi2s1.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
  hi2s1.Init.AudioFreq = I2S_AUDIOFREQ_48K;
  hi2s1.Init.CPOL = I2S_CPOL_LOW;
  hi2s1.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s1.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
  if (HAL_I2S_Init(&hi2s1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S1_Init 2 */

  /* USER CODE END I2S1_Init 2 */

}

/**
  * @brief I2S2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2S2_Init(void)
{

  /* USER CODE BEGIN I2S2_Init 0 */

  /* USER CODE END I2S2_Init 0 */

  /* USER CODE BEGIN I2S2_Init 1 */

  /* USER CODE END I2S2_Init 1 */
  hi2s2.Instance = SPI2;
  hi2s2.Init.Mode = I2S_MODE_MASTER_RX;
  hi2s2.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s2.Init.DataFormat = I2S_DATAFORMAT_32B;
  hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
  hi2s2.Init.AudioFreq = I2S_AUDIOFREQ_48K;
  hi2s2.Init.CPOL = I2S_CPOL_LOW;
  hi2s2.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s2.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
  if (HAL_I2S_Init(&hi2s2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S2_Init 2 */

  /* USER CODE END I2S2_Init 2 */

}

/**
  * @brief I2S4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2S4_Init(void)
{

  /* USER CODE BEGIN I2S4_Init 0 */

  /* USER CODE END I2S4_Init 0 */

  /* USER CODE BEGIN I2S4_Init 1 */

  /* USER CODE END I2S4_Init 1 */
  hi2s4.Instance = SPI4;
  hi2s4.Init.Mode = I2S_MODE_SLAVE_RX;
  hi2s4.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s4.Init.DataFormat = I2S_DATAFORMAT_32B;
  hi2s4.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
  hi2s4.Init.AudioFreq = I2S_AUDIOFREQ_48K;
  hi2s4.Init.CPOL = I2S_CPOL_LOW;
  hi2s4.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s4.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
  if (HAL_I2S_Init(&hi2s4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S4_Init 2 */

  /* USER CODE END I2S4_Init 2 */

}

/**
  * @brief I2S5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2S5_Init(void)
{

  /* USER CODE BEGIN I2S5_Init 0 */

  /* USER CODE END I2S5_Init 0 */

  /* USER CODE BEGIN I2S5_Init 1 */

  /* USER CODE END I2S5_Init 1 */
  hi2s5.Instance = SPI5;
  hi2s5.Init.Mode = I2S_MODE_SLAVE_RX;
  hi2s5.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s5.Init.DataFormat = I2S_DATAFORMAT_32B;
  hi2s5.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
  hi2s5.Init.AudioFreq = I2S_AUDIOFREQ_48K;
  hi2s5.Init.CPOL = I2S_CPOL_LOW;
  hi2s5.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s5.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
  if (HAL_I2S_Init(&hi2s5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S5_Init 2 */

  /* USER CODE END I2S5_Init 2 */

}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
  /* DMA2_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/**
  * @brief  Initialize UAC Microphone
  * @author  Nhan Vo
  * @date    2025-09-14
  * @retval None
  */
void UAC_Init_Microphone(void)
{
  HAL_StatusTypeDef status;
  
  /* Configure audio parameters for microphone */
  audio_config.format = UAC_FORMAT_PCM;
  audio_config.sample_rate = UAC_SAMPLE_RATE_48K;
  audio_config.channels = 1;  /* Mono microphone */
  audio_config.bits_per_sample = 16;
  audio_config.frame_size = 2; /* 16-bit = 2 bytes per sample */
  
  /* Initialize UAC device */
  status = UAC_Init(&huac, &hUsbDeviceFS, &audio_config);
  if (status == HAL_OK) {
    printf("UAC Init successful\r\n");
    
    /* Set audio data callback (optional) */
    UAC_SetAudioDataCallback(&huac, NULL);
    
    /* Start audio streaming */
    status = UAC_StartStreaming(&huac);
    if (status == HAL_OK) {
      uac_initialized = 1;
      printf("UAC Microphone initialized successfully!\r\n");
      printf("UAC is_configured: %d, is_streaming: %d\r\n", huac.is_configured, huac.is_streaming);
      
      /* Start audio test with sine wave */
      UAC_StartAudioTest(&huac, test_mode);
      
      /* Send test message via UART */
      uint8_t msg[] = "UAC Microphone initialized successfully!\r\n";
      HAL_UART_Transmit(&huart2, msg, sizeof(msg)-1, 1000);
    } else {
      printf("UAC StartStreaming failed: %d\r\n", status);
    }
  }
}

/**
  * @brief  UAC Audio Test Function
  * @author  Nhan Vo
  * @date    2025-09-14
  * @retval None
  */
void UAC_Test_Audio(void)
{
  static uint32_t last_test_time = 0;
  static uint8_t test_cycle = 0;
  uint32_t current_time = HAL_GetTick();
  
  /* Change test mode every 5 seconds for demonstration */
  if ((current_time - last_test_time) >= 5000) {
    last_test_time = current_time;
    test_cycle++;
    
    switch (test_cycle % 4) {
      case 0:
        test_mode = 0; /* Silence */
        UAC_StartAudioTest(&huac, test_mode);
        break;
      case 1:
        test_mode = 1; /* Sine wave 1kHz */
        UAC_GenerateTestSignal(&huac, 1000, 16000, 0); /* 1kHz, 50% amplitude, continuous */
        break;
      case 2:
        test_mode = 2; /* Square wave 2kHz */
        UAC_GenerateTestSignal(&huac, 2000, 16000, 0); /* 2kHz, 50% amplitude, continuous */
        break;
      case 3:
        test_mode = 3; /* Noise */
        UAC_StartAudioTest(&huac, test_mode);
        break;
    }
    
    /* Send test mode change message via UART */
    uint8_t msg[] = "Test mode changed\r\n";
    HAL_UART_Transmit(&huart2, msg, sizeof(msg)-1, 100);
  }
  
  /* Process audio test data */
  UAC_ProcessAudioTest(&huac);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}

/* USER CODE BEGIN 7 */
/**
  * @brief  Initialize Mic Array
  * @author Nhan Vo
  * @date 2025-01-14
  * @retval None
  */
void MIC_ARRAY_Init_Microphones(void)
{
  HAL_StatusTypeDef status;
  
  printf("[MAIN] Starting MIC_ARRAY_Init_Microphones...\r\n");
  
  /* Configure mic array parameters */
  mic_array_config.sample_rate = 48000;  /* 48kHz sample rate */
  mic_array_config.channels = 4;         /* 4 microphones (MIC_D0, D1, D2, D3) */
  mic_array_config.bits_per_sample = 16; /* 16-bit samples */
  mic_array_config.buffer_size = MIC_ARRAY_BUFFER_SIZE;
  mic_array_config.mic_count = 4;        /* 4 microphones on Sipeed Mic Array */
  
  printf("[MAIN] Config: sample_rate=%d, channels=%d, bits=%d, buffer_size=%d\r\n",
         mic_array_config.sample_rate, mic_array_config.channels, 
         mic_array_config.bits_per_sample, mic_array_config.buffer_size);
  
  /* Initialize Mic Array */
  printf("[MAIN] Calling MIC_ARRAY_Init...\r\n");
  status = MIC_ARRAY_Init(&hmic_array, &hi2s2, &mic_array_config);
  printf("[MAIN] MIC_ARRAY_Init returned: %d\r\n", status);
  
  if (status == HAL_OK) {
    printf("Mic Array Init successful\r\n");
    
    /* Start Mic Array streaming */
    printf("[MAIN] Calling MIC_ARRAY_StartStreaming...\r\n");
    status = MIC_ARRAY_StartStreaming(&hmic_array);
    printf("[MAIN] MIC_ARRAY_StartStreaming returned: %d\r\n", status);
    
    if (status == HAL_OK) {
      mic_array_initialized = 1;
      printf("Mic Array streaming started successfully!\r\n");
      printf("Mic Array channels: %d, sample_rate: %d\r\n", 
             hmic_array.channels, hmic_array.sample_rate);
    } else {
      printf("Mic Array StartStreaming failed: %d\r\n", status);
    }
  } else {
    printf("Mic Array Init failed: %d\r\n", status);
  }
  
  printf("[MAIN] MIC_ARRAY_Init_Microphones completed\r\n");
}

/**
  * @brief  Process Mic Array Audio Data
  * @author Nhan Vo
  * @date 2025-01-14
  * @retval None
  */
void MIC_ARRAY_Process_Audio(void)
{
  static uint32_t debug_counter = 0;
  
  if (!mic_array_initialized) {
    return;
  }
  
  /* Read data from mic array - only when DMA has new data */
  HAL_StatusTypeDef status = MIC_ARRAY_ReadData(&hmic_array, mic_audio_buffer, MIC_ARRAY_BUFFER_SIZE);
  if (status == HAL_OK) {
    /* Debug: Check if we're getting non-zero data */
    uint32_t non_zero_count = 0;
    for (uint16_t i = 0; i < MIC_ARRAY_BUFFER_SIZE; i++) {
      if (mic_audio_buffer[i] != 0) {
        non_zero_count++;
      }
    }
    
    /* Debug output every 10 calls when we have data */
    if (debug_counter % 10 == 0) {
      printf("[MIC_ARRAY] Processing: Non-zero samples: %lu/%d\r\n", non_zero_count, MIC_ARRAY_BUFFER_SIZE);
      if (non_zero_count > 0) {
        printf("[MIC_ARRAY] Sample values: %d, %d, %d, %d\r\n", 
               mic_audio_buffer[0], mic_audio_buffer[1], mic_audio_buffer[2], mic_audio_buffer[3]);
      }
    }
    debug_counter++;
    
    /* Process multi-channel data to mono */
    status = MIC_ARRAY_ProcessData(&hmic_array, mic_audio_buffer, usb_audio_buffer, MIC_ARRAY_BUFFER_SIZE);
    if (status == HAL_OK) {
      /* Data is now ready in usb_audio_buffer for USB transmission */
      printf("[MIC_ARRAY] Processed %d samples to USB buffer\r\n", MIC_ARRAY_BUFFER_SIZE / 4);
    }
  } else if (status == HAL_BUSY) {
    /* No new DMA data - this is normal */
  } else {
    /* Error occurred */
    if (debug_counter % 100 == 0) {
      printf("[MIC_ARRAY] ReadData error: %d\r\n", status);
    }
    debug_counter++;
  }
}
/* USER CODE END 7 */

#endif /* USE_FULL_ASSERT */
