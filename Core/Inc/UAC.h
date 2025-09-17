/**
  ******************************************************************************
  * @file    UAC.h
  * @brief   USB Audio Class (UAC) Driver Header File
  * @author  Nhan Vo
  * @version 1.0
  * @date    2025-09-14
  ******************************************************************************
  * @attention
  *
  * This driver implements USB Audio Class 1.0 specification for microphone input.
  * It provides a reusable library for STM32 microcontrollers to act as USB audio devices.
  *
  ******************************************************************************
  */

#ifndef __UAC_H
#define __UAC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_audio.h"

/* Exported types ------------------------------------------------------------*/

/**
  * @brief  UAC Audio Format Type
  */
typedef enum {
    UAC_FORMAT_PCM = 0x01,
    UAC_FORMAT_PCM8 = 0x02,
    UAC_FORMAT_IEEE_FLOAT = 0x03,
    UAC_FORMAT_ALAW = 0x04,
    UAC_FORMAT_MULAW = 0x05
} UAC_AudioFormat_t;

/**
  * @brief  UAC Sample Rate Type
  */
typedef enum {
    UAC_SAMPLE_RATE_8K = 8000,
    UAC_SAMPLE_RATE_16K = 16000,
    UAC_SAMPLE_RATE_22K = 22050,
    UAC_SAMPLE_RATE_44K = 44100,
    UAC_SAMPLE_RATE_48K = 48000,
    UAC_SAMPLE_RATE_96K = 96000
} UAC_SampleRate_t;

/**
  * @brief  UAC Audio Configuration Structure
  */
typedef struct {
    UAC_AudioFormat_t format;
    UAC_SampleRate_t sample_rate;
    uint8_t channels;
    uint8_t bits_per_sample;
    uint16_t frame_size;
} UAC_AudioConfig_t;

/* Audio Data Endpoint Constants */
#define UAC_AUDIO_EP_SIZE                 64
#define UAC_AUDIO_BUFFER_SIZE             256U

/**
  * @brief  UAC Device Handle Structure
  */
typedef struct {
    USBD_HandleTypeDef *pdev;
    UAC_AudioConfig_t audio_config;
    uint8_t is_configured;
    uint8_t is_streaming;
    uint16_t audio_buffer[UAC_AUDIO_BUFFER_SIZE];
    uint16_t buffer_index;
    void (*audio_data_callback)(uint16_t *data, uint16_t length);
} UAC_HandleTypeDef;

/* Exported constants --------------------------------------------------------*/

/* USB Audio Class Constants */
#define UAC_AUDIO_CLASS                   0x01
#define UAC_AUDIO_SUBCLASS_CONTROL        0x01
#define UAC_AUDIO_SUBCLASS_STREAMING      0x02
#define UAC_AUDIO_PROTOCOL_UNDEFINED      0x00

/* Audio Interface Descriptor Constants */
#define UAC_AUDIO_INTERFACE               0x00
#define UAC_AUDIO_STREAMING_INTERFACE     0x01

/* Audio Control Interface Constants */
#define UAC_AC_INTERFACE_HEADER           0x01
#define UAC_AC_INTERFACE_INPUT_TERMINAL   0x02
#define UAC_AC_INTERFACE_OUTPUT_TERMINAL  0x03
#define UAC_AC_INTERFACE_FEATURE_UNIT     0x06

/* Audio Streaming Interface Constants */
#define UAC_AS_INTERFACE_GENERAL          0x01
#define UAC_AS_INTERFACE_FORMAT_TYPE      0x02

/* Audio Terminal Types */
#define UAC_TERMINAL_INPUT_MICROPHONE     0x0201
#define UAC_TERMINAL_OUTPUT_USB_STREAMING 0x0101

/* Audio Format Types */
#define UAC_FORMAT_TYPE_I                 0x01

/* Audio Control Constants */
#define UAC_MUTE_CONTROL                  0x0001
#define UAC_VOLUME_CONTROL                0x0002
#define UAC_BASS_CONTROL                  0x0003
#define UAC_MID_CONTROL                   0x0004
#define UAC_TREBLE_CONTROL                0x0005
#define UAC_GRAPHIC_EQUALIZER_CONTROL     0x0006
#define UAC_AUTOMATIC_GAIN_CONTROL        0x0007
#define UAC_DELAY_CONTROL                 0x0008
#define UAC_BASS_BOOST_CONTROL            0x0009
#define UAC_LOUDNESS_CONTROL              0x000A

/* Exported macros -----------------------------------------------------------*/

/* Audio Data Conversion Macros */
#define UAC_CONVERT_16BIT_TO_8BIT(data)   ((uint8_t)((data >> 8) + 128))
#define UAC_CONVERT_8BIT_TO_16BIT(data)   ((uint16_t)((data - 128) << 8))

/* Buffer Management Macros */
#define UAC_BUFFER_EMPTY(handle)           ((handle)->buffer_index == 0)
#define UAC_BUFFER_FULL(handle)            ((handle)->buffer_index >= UAC_AUDIO_BUFFER_SIZE)

/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  Initialize UAC device
  * @param  huac: UAC handle pointer
  * @param  pdev: USB device handle pointer
  * @param  config: Audio configuration pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_Init(UAC_HandleTypeDef *huac, USBD_HandleTypeDef *pdev, UAC_AudioConfig_t *config);

/**
  * @brief  Deinitialize UAC device
  * @param  huac: UAC handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_DeInit(UAC_HandleTypeDef *huac);

/**
  * @brief  Start audio streaming
  * @param  huac: UAC handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_StartStreaming(UAC_HandleTypeDef *huac);

/**
  * @brief  Stop audio streaming
  * @param  huac: UAC handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_StopStreaming(UAC_HandleTypeDef *huac);

/**
  * @brief  Process audio data
  * @param  huac: UAC handle pointer
  * @param  data: Audio data pointer
  * @param  length: Data length
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_ProcessAudioData(UAC_HandleTypeDef *huac, uint16_t *data, uint16_t length);

/**
  * @brief  Set audio configuration
  * @param  huac: UAC handle pointer
  * @param  config: Audio configuration pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_SetAudioConfig(UAC_HandleTypeDef *huac, UAC_AudioConfig_t *config);

/**
  * @brief  Get audio configuration
  * @param  huac: UAC handle pointer
  * @param  config: Audio configuration pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_GetAudioConfig(UAC_HandleTypeDef *huac, UAC_AudioConfig_t *config);

/**
  * @brief  Set audio data callback
  * @param  huac: UAC handle pointer
  * @param  callback: Callback function pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_SetAudioDataCallback(UAC_HandleTypeDef *huac, void (*callback)(uint16_t *data, uint16_t length));

/**
  * @brief  Handle USB audio control requests
  * @param  huac: UAC handle pointer
  * @param  req: USB request pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_HandleControlRequest(UAC_HandleTypeDef *huac, USBD_SetupReqTypedef *req);

/**
  * @brief  Handle USB audio streaming requests
  * @param  huac: UAC handle pointer
  * @param  req: USB request pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_HandleStreamingRequest(UAC_HandleTypeDef *huac, USBD_SetupReqTypedef *req);

/**
  * @brief  Get USB audio descriptor
  * @param  speed: USB speed
  * @param  length: Descriptor length pointer
  * @retval Descriptor pointer
  */
uint8_t* UAC_GetDescriptor(uint8_t speed, uint16_t *length);

/**
  * @brief  Get USB audio configuration descriptor
  * @param  speed: USB speed
  * @param  length: Descriptor length pointer
  * @retval Descriptor pointer
  */
uint8_t* UAC_GetConfigDescriptor(uint8_t speed, uint16_t *length);

/**
  * @brief  Get USB audio string descriptor
  * @param  speed: USB speed
  * @param  length: Descriptor length pointer
  * @retval Descriptor pointer
  */
uint8_t* UAC_GetStringDescriptor(uint8_t speed, uint8_t index, uint16_t *length);

/**
  * @brief  Get USB audio device qualifier descriptor
  * @param  length: Descriptor length pointer
  * @retval Descriptor pointer
  */
uint8_t* UAC_GetDeviceQualifierDescriptor(uint16_t *length);

/**
  * @brief  USB audio data IN callback
  * @param  huac: UAC handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_DataIn(UAC_HandleTypeDef *huac);

/**
  * @brief  USB audio data OUT callback
  * @param  huac: UAC handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_DataOut(UAC_HandleTypeDef *huac);

/**
  * @brief  USB audio SOF callback
  * @param  huac: UAC handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_SOF(UAC_HandleTypeDef *huac);

/**
  * @brief  USB audio reset callback
  * @param  huac: UAC handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_Reset(UAC_HandleTypeDef *huac);

/**
  * @brief  USB audio suspend callback
  * @param  huac: UAC handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_Suspend(UAC_HandleTypeDef *huac);

/**
  * @brief  USB audio resume callback
  * @param  huac: UAC handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_Resume(UAC_HandleTypeDef *huac);

/**
  * @brief  Generate test audio signal (sine wave)
  * @param  huac: UAC handle pointer
  * @param  frequency: Test frequency in Hz
  * @param  amplitude: Test amplitude (0-32767)
  * @param  duration_ms: Test duration in milliseconds (0 = continuous)
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_GenerateTestSignal(UAC_HandleTypeDef *huac, uint32_t frequency, uint16_t amplitude, uint32_t duration_ms);

/**
  * @brief  Generate test audio data for testing
  * @param  huac: UAC handle pointer
  * @param  test_mode: Test mode (0=silence, 1=sine wave, 2=square wave, 3=noise)
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_StartAudioTest(UAC_HandleTypeDef *huac, uint8_t test_mode);

/**
  * @brief  Stop audio test
  * @param  huac: UAC handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_StopAudioTest(UAC_HandleTypeDef *huac);

/**
  * @brief  Process audio test data (call in main loop)
  * @param  huac: UAC handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_ProcessAudioTest(UAC_HandleTypeDef *huac);

/**
  * @brief  Generate sine wave test sound
  * @param  huac: UAC handle pointer
  * @param  frequency: Frequency in Hz
  * @param  amplitude: Amplitude (0-100)
  * @retval None
  */
void UAC_GenerateSineWave(UAC_HandleTypeDef *huac, uint16_t frequency, uint8_t amplitude);

/**
  * @brief  Generate square wave test sound
  * @param  huac: UAC handle pointer
  * @param  frequency: Frequency in Hz
  * @param  amplitude: Amplitude (0-100)
  * @retval None
  */
void UAC_GenerateSquareWave(UAC_HandleTypeDef *huac, uint16_t frequency, uint8_t amplitude);

/**
  * @brief  Generate noise test sound
  * @param  huac: UAC handle pointer
  * @param  amplitude: Amplitude (0-100)
  * @retval None
  */
void UAC_GenerateNoise(UAC_HandleTypeDef *huac, uint8_t amplitude);

/**
  * @brief  Generate silence
  * @param  huac: UAC handle pointer
  * @retval None
  */
void UAC_GenerateSilence(UAC_HandleTypeDef *huac);

/**
  * @brief  Test sound sequence (different sounds in sequence)
  * @param  huac: UAC handle pointer
  * @retval None
  */
void UAC_TestSoundSequence(UAC_HandleTypeDef *huac);

#ifdef __cplusplus
}
#endif

#endif /* __UAC_H */
