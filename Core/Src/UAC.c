/**
  ******************************************************************************
  * @file    UAC.c
  * @brief   USB Audio Class (UAC) Driver Implementation
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

/* Includes ------------------------------------------------------------------*/
#include "UAC.h"
#include "usbd_audio_if.h"
#include <math.h>
#include <stdlib.h>

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define UAC_GET_CUR                        0x81
#define UAC_SET_CUR                        0x01
#define UAC_GET_MIN                        0x82
#define UAC_SET_MIN                        0x02
#define UAC_GET_MAX                        0x83
#define UAC_SET_MAX                        0x03
#define UAC_GET_RES                        0x84
#define UAC_SET_RES                        0x04

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Audio Test Variables */
static uint8_t uac_test_mode = 0;
static uint32_t uac_test_frequency = 1000;  /* 1kHz default */
static uint16_t uac_test_amplitude = 16000; /* 50% amplitude */
static uint32_t uac_test_phase = 0;
static uint32_t uac_test_duration = 0;
static uint32_t uac_test_start_time = 0;
static uint8_t uac_test_active = 0;

/* USB Audio Class Descriptors */
static const uint8_t UAC_DeviceDescriptor[18] = {
    0x12,                       /* bLength */
    0x01,                       /* bDescriptorType (Device) */
    0x00, 0x02,                 /* bcdUSB */
    0x00,                       /* bDeviceClass */
    0x00,                       /* bDeviceSubClass */
    0x00,                       /* bDeviceProtocol */
    0x40,                       /* bMaxPacketSize */
    0x34, 0x12,                 /* idVendor */
    0x01, 0x00,                 /* idProduct */
    0x00, 0x01,                 /* bcdDevice */
    0x01,                       /* iManufacturer */
    0x02,                       /* iProduct */
    0x03,                       /* iSerialNumber */
    0x01                        /* bNumConfigurations */
};

static const uint8_t UAC_ConfigDescriptor[109] = {
    /* Configuration Descriptor */
    0x09,                       /* bLength */
    USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType */
    0x6D, 0x00,                   /* wTotalLength (109 bytes) */
    0x02,                       /* bNumInterfaces */
    0x01,                       /* bConfigurationValue */
    0x00,                       /* iConfiguration */
    0xC0,                       /* bmAttributes */
    0x32,                       /* bMaxPower */

    /* Audio Control Interface Descriptor */
    0x09,                       /* bLength */
    USB_DESC_TYPE_INTERFACE,    /* bDescriptorType */
    0x00,                       /* bInterfaceNumber */
    0x00,                       /* bAlternateSetting */
    0x00,                       /* bNumEndpoints */
    UAC_AUDIO_CLASS,            /* bInterfaceClass */
    UAC_AUDIO_SUBCLASS_CONTROL, /* bInterfaceSubClass */
    UAC_AUDIO_PROTOCOL_UNDEFINED, /* bInterfaceProtocol */
    0x00,                       /* iInterface */

    /* Audio Control Header Descriptor */
    0x09,                       /* bLength */
    UAC_AC_INTERFACE_HEADER,    /* bDescriptorType */
    0x00, 0x01,                 /* bcdADC */
    0x27, 0x00,                 /* wTotalLength */
    0x01,                       /* bInCollection */
    0x01,                       /* baInterfaceNr */

    /* Input Terminal Descriptor - Microphone */
    0x0C,                       /* bLength */
    UAC_AC_INTERFACE_INPUT_TERMINAL, /* bDescriptorType */
    0x01,                       /* bTerminalID */
    0x01, 0x02,                 /* wTerminalType - Microphone (0x0201) */
    0x00,                       /* bAssocTerminal */
    0x01,                       /* bNrChannels */
    0x00, 0x00,                 /* wChannelConfig */
    0x00,                       /* iChannelNames */
    0x00,                       /* iTerminal */

    /* Output Terminal Descriptor - USB Streaming */
    0x09,                       /* bLength */
    UAC_AC_INTERFACE_OUTPUT_TERMINAL, /* bDescriptorType */
    0x02,                       /* bTerminalID */
    0x01, 0x01,                 /* wTerminalType - USB Streaming (0x0101) */
    0x00,                       /* bAssocTerminal */
    0x01,                       /* bSourceID */
    0x00,                       /* iTerminal */

    /* Audio Streaming Interface Descriptor */
    0x09,                       /* bLength */
    USB_DESC_TYPE_INTERFACE,    /* bDescriptorType */
    0x01,                       /* bInterfaceNumber */
    0x00,                       /* bAlternateSetting */
    0x00,                       /* bNumEndpoints */
    UAC_AUDIO_CLASS,            /* bInterfaceClass */
    UAC_AUDIO_SUBCLASS_STREAMING, /* bInterfaceSubClass */
    UAC_AUDIO_PROTOCOL_UNDEFINED, /* bInterfaceProtocol */
    0x00,                       /* iInterface */

    /* Audio Streaming Interface Descriptor - Alternate Setting 1 */
    0x09,                       /* bLength */
    USB_DESC_TYPE_INTERFACE,    /* bDescriptorType */
    0x01,                       /* bInterfaceNumber */
    0x01,                       /* bAlternateSetting */
    0x01,                       /* bNumEndpoints */
    UAC_AUDIO_CLASS,            /* bInterfaceClass */
    UAC_AUDIO_SUBCLASS_STREAMING, /* bInterfaceSubClass */
    UAC_AUDIO_PROTOCOL_UNDEFINED, /* bInterfaceProtocol */
    0x00,                       /* iInterface */

    /* Audio Streaming General Descriptor */
    0x07,                       /* bLength */
    UAC_AS_INTERFACE_GENERAL,   /* bDescriptorType */
    0x01,                       /* bTerminalLink - Link to Input Terminal */
    0x00,                       /* bDelay */
    0x01, 0x00,                 /* wFormatTag - PCM */

    /* Audio Streaming Format Type Descriptor */
    0x0B,                       /* bLength */
    UAC_AS_INTERFACE_FORMAT_TYPE, /* bDescriptorType */
    UAC_FORMAT_TYPE_I,          /* bFormatType */
    0x01,                       /* bNrChannels */
    0x02,                       /* bSubframeSize */
    0x10,                       /* bBitResolution */
    0x01,                       /* bSamFreqType */
    0x80, 0xBB, 0x00,           /* tSamFreq */

    /* Audio Data Endpoint Descriptor - Input for Microphone */
    0x07,                       /* bLength */
    USB_DESC_TYPE_ENDPOINT,     /* bDescriptorType */
    AUDIO_OUT_EP,               /* bEndpointAddress - IN endpoint for microphone */
    0x05,                       /* bmAttributes - Isochronous */
    LOBYTE(UAC_AUDIO_EP_SIZE), HIBYTE(UAC_AUDIO_EP_SIZE), /* wMaxPacketSize */
    0x01                        /* bInterval */
};

static const uint8_t UAC_StringLangID[4] = {
    0x04,                       /* bLength */
    USB_DESC_TYPE_STRING,       /* bDescriptorType */
    0x09, 0x04                  /* wLANGID */
};

static const uint8_t UAC_StringVendor[] = {
    0x24,                       /* bLength */
    USB_DESC_TYPE_STRING,       /* bDescriptorType */
    'S', 0, 'T', 0, 'M', 0, 'i', 0, 'c', 0, 'r', 0, 'o', 0, 'e', 0,
    'l', 0, 'e', 0, 'c', 0, 't', 0, 'r', 0, 'o', 0, 'n', 0, 'i', 0,
    'c', 0, 's', 0
};

static const uint8_t UAC_StringProduct[] = {
    0x28,                       /* bLength */
    USB_DESC_TYPE_STRING,       /* bDescriptorType */
    'S', 0, 'T', 0, 'M', 0, '3', 0, '2', 0, ' ', 0, 'U', 0, 'A', 0,
    'C', 0, ' ', 0, 'M', 0, 'i', 0, 'c', 0, 'r', 0, 'o', 0, 'p', 0,
    'h', 0, 'o', 0, 'n', 0, 'e', 0
};

static const uint8_t UAC_StringSerial[] = {
    0x1C,                       /* bLength */
    USB_DESC_TYPE_STRING,       /* bDescriptorType */
    '1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0, '8', 0,
    '9', 0, '0', 0, '1', 0, '2', 0, '3', 0
};

static const uint8_t UAC_StringInterface[] = {
    0x1E,                       /* bLength */
    USB_DESC_TYPE_STRING,       /* bDescriptorType */
    'A', 0, 'u', 0, 'd', 0, 'i', 0, 'o', 0, ' ', 0, 'I', 0, 'n', 0,
    't', 0, 'e', 0, 'r', 0, 'f', 0, 'a', 0, 'c', 0, 'e', 0
};

static const uint8_t* UAC_StringTable[5] = {
    UAC_StringLangID,
    UAC_StringVendor,
    UAC_StringProduct,
    UAC_StringSerial,
    UAC_StringInterface
};

/* Private function prototypes -----------------------------------------------*/
static void UAC_ProcessAudioBuffer(UAC_HandleTypeDef *huac);
static void UAC_SendAudioData(UAC_HandleTypeDef *huac);

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Initialize UAC device
  * @param  huac: UAC handle pointer
  * @param  pdev: USB device handle pointer
  * @param  config: Audio configuration pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_Init(UAC_HandleTypeDef *huac, USBD_HandleTypeDef *pdev, UAC_AudioConfig_t *config)
{
    if (huac == NULL || pdev == NULL || config == NULL) {
        return HAL_ERROR;
    }

    /* Initialize handle */
    huac->pdev = pdev;
    huac->is_configured = 1;  /* Set to 1 to enable UAC functionality */
    huac->is_streaming = 0;
    huac->buffer_index = 0;
    huac->audio_data_callback = NULL;

    /* Set default audio configuration */
    huac->audio_config.format = config->format;
    huac->audio_config.sample_rate = config->sample_rate;
    huac->audio_config.channels = config->channels;
    huac->audio_config.bits_per_sample = config->bits_per_sample;
    huac->audio_config.frame_size = config->frame_size;

    /* Clear audio buffer */
    memset(huac->audio_buffer, 0, sizeof(huac->audio_buffer));

    return HAL_OK;
}

/**
  * @brief  Deinitialize UAC device
  * @param  huac: UAC handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_DeInit(UAC_HandleTypeDef *huac)
{
    if (huac == NULL) {
        return HAL_ERROR;
    }

    /* Stop streaming if active */
    UAC_StopStreaming(huac);

    /* Clear handle */
    huac->pdev = NULL;
    huac->is_configured = 0;
    huac->is_streaming = 0;
    huac->buffer_index = 0;
    huac->audio_data_callback = NULL;

    /* Clear audio buffer */
    memset(huac->audio_buffer, 0, sizeof(huac->audio_buffer));

    return HAL_OK;
}

/**
  * @brief  Start audio streaming
  * @param  huac: UAC handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_StartStreaming(UAC_HandleTypeDef *huac)
{
    if (huac == NULL || huac->pdev == NULL) {
        return HAL_ERROR;
    }

    if (!huac->is_configured) {
        return HAL_ERROR;
    }

    huac->is_streaming = 1;
    huac->buffer_index = 0;

    return HAL_OK;
}

/**
  * @brief  Stop audio streaming
  * @param  huac: UAC handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_StopStreaming(UAC_HandleTypeDef *huac)
{
    if (huac == NULL) {
        return HAL_ERROR;
    }

    huac->is_streaming = 0;
    huac->buffer_index = 0;

    return HAL_OK;
}

/**
  * @brief  Process audio data
  * @param  huac: UAC handle pointer
  * @param  data: Audio data pointer
  * @param  length: Data length
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_ProcessAudioData(UAC_HandleTypeDef *huac, uint16_t *data, uint16_t length)
{
    if (huac == NULL || data == NULL || length == 0) {
        return HAL_ERROR;
    }

    if (!huac->is_streaming) {
        return HAL_ERROR;
    }

    /* Copy data to buffer */
    for (uint16_t i = 0; i < length && huac->buffer_index < UAC_AUDIO_BUFFER_SIZE; i++) {
        huac->audio_buffer[huac->buffer_index++] = data[i];
    }

    /* Process buffer if full */
    if (UAC_BUFFER_FULL(huac)) {
        UAC_ProcessAudioBuffer(huac);
    }

    return HAL_OK;
}

/**
  * @brief  Set audio configuration
  * @param  huac: UAC handle pointer
  * @param  config: Audio configuration pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_SetAudioConfig(UAC_HandleTypeDef *huac, UAC_AudioConfig_t *config)
{
    if (huac == NULL || config == NULL) {
        return HAL_ERROR;
    }

    huac->audio_config.format = config->format;
    huac->audio_config.sample_rate = config->sample_rate;
    huac->audio_config.channels = config->channels;
    huac->audio_config.bits_per_sample = config->bits_per_sample;
    huac->audio_config.frame_size = config->frame_size;

    return HAL_OK;
}

/**
  * @brief  Get audio configuration
  * @param  huac: UAC handle pointer
  * @param  config: Audio configuration pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_GetAudioConfig(UAC_HandleTypeDef *huac, UAC_AudioConfig_t *config)
{
    if (huac == NULL || config == NULL) {
        return HAL_ERROR;
    }

    config->format = huac->audio_config.format;
    config->sample_rate = huac->audio_config.sample_rate;
    config->channels = huac->audio_config.channels;
    config->bits_per_sample = huac->audio_config.bits_per_sample;
    config->frame_size = huac->audio_config.frame_size;

    return HAL_OK;
}

/**
  * @brief  Set audio data callback
  * @param  huac: UAC handle pointer
  * @param  callback: Callback function pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_SetAudioDataCallback(UAC_HandleTypeDef *huac, void (*callback)(uint16_t *data, uint16_t length))
{
    if (huac == NULL) {
        return HAL_ERROR;
    }

    huac->audio_data_callback = callback;
    return HAL_OK;
}

/**
  * @brief  Handle USB audio control requests
  * @param  huac: UAC handle pointer
  * @param  req: USB request pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_HandleControlRequest(UAC_HandleTypeDef *huac, USBD_SetupReqTypedef *req)
{
    if (huac == NULL || req == NULL) {
        return HAL_ERROR;
    }

    switch (req->bmRequest & USB_REQ_TYPE_MASK) {
        case USB_REQ_TYPE_CLASS:
            switch (req->bRequest) {
                case UAC_GET_CUR:
                case UAC_GET_MIN:
                case UAC_GET_MAX:
                case UAC_GET_RES:
                    /* Handle audio control requests */
                    break;
                case UAC_SET_CUR:
                case UAC_SET_MIN:
                case UAC_SET_MAX:
                case UAC_SET_RES:
                    /* Handle audio control requests */
                    break;
                default:
                    return HAL_ERROR;
            }
            break;
        default:
            return HAL_ERROR;
    }

    return HAL_OK;
}

/**
  * @brief  Handle USB audio streaming requests
  * @param  huac: UAC handle pointer
  * @param  req: USB request pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_HandleStreamingRequest(UAC_HandleTypeDef *huac, USBD_SetupReqTypedef *req)
{
    if (huac == NULL || req == NULL) {
        return HAL_ERROR;
    }

    switch (req->bmRequest & USB_REQ_TYPE_MASK) {
        case USB_REQ_TYPE_CLASS:
            switch (req->bRequest) {
                case UAC_GET_CUR:
                    /* Handle streaming control requests */
                    break;
                case UAC_SET_CUR:
                    /* Handle streaming control requests */
                    break;
                default:
                    return HAL_ERROR;
            }
            break;
        default:
            return HAL_ERROR;
    }

    return HAL_OK;
}

/**
  * @brief  Get USB audio descriptor
  * @param  speed: USB speed
  * @param  length: Descriptor length pointer
  * @retval Descriptor pointer
  */
uint8_t* UAC_GetDescriptor(uint8_t speed, uint16_t *length)
{
    *length = sizeof(UAC_DeviceDescriptor);
    return (uint8_t*)UAC_DeviceDescriptor;
}

/**
  * @brief  Get USB audio configuration descriptor
  * @param  speed: USB speed
  * @param  length: Descriptor length pointer
  * @retval Descriptor pointer
  */
uint8_t* UAC_GetConfigDescriptor(uint8_t speed, uint16_t *length)
{
    *length = sizeof(UAC_ConfigDescriptor);
    return (uint8_t*)UAC_ConfigDescriptor;
}

/**
  * @brief  Get USB audio string descriptor
  * @param  speed: USB speed
  * @param  length: Descriptor length pointer
  * @retval Descriptor pointer
  */
uint8_t* UAC_GetStringDescriptor(uint8_t speed, uint8_t index, uint16_t *length)
{
    if (index >= 5) {
        return NULL;
    }

    *length = UAC_StringTable[index][0];
    return (uint8_t*)UAC_StringTable[index];
}

/**
  * @brief  Get USB audio device qualifier descriptor
  * @param  length: Descriptor length pointer
  * @retval Descriptor pointer
  */
uint8_t* UAC_GetDeviceQualifierDescriptor(uint16_t *length)
{
    *length = 0;
    return NULL;
}

/**
  * @brief  USB audio data IN callback
  * @param  huac: UAC handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_DataIn(UAC_HandleTypeDef *huac)
{
    if (huac == NULL) {
        return HAL_ERROR;
    }

    /* Send next audio data if available */
    UAC_SendAudioData(huac);

    return HAL_OK;
}

/**
  * @brief  USB audio data OUT callback
  * @param  huac: UAC handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_DataOut(UAC_HandleTypeDef *huac)
{
    if (huac == NULL) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
  * @brief  USB audio SOF callback
  * @param  huac: UAC handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_SOF(UAC_HandleTypeDef *huac)
{
    if (huac == NULL) {
        return HAL_ERROR;
    }

    /* Process audio buffer on SOF */
    if (huac->is_streaming) {
        UAC_ProcessAudioBuffer(huac);
    }

    return HAL_OK;
}

/**
  * @brief  USB audio reset callback
  * @param  huac: UAC handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_Reset(UAC_HandleTypeDef *huac)
{
    if (huac == NULL) {
        return HAL_ERROR;
    }

    huac->is_configured = 0;
    huac->is_streaming = 0;
    huac->buffer_index = 0;

    return HAL_OK;
}

/**
  * @brief  USB audio suspend callback
  * @param  huac: UAC handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_Suspend(UAC_HandleTypeDef *huac)
{
    if (huac == NULL) {
        return HAL_ERROR;
    }

    huac->is_streaming = 0;

    return HAL_OK;
}

/**
  * @brief  USB audio resume callback
  * @param  huac: UAC handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_Resume(UAC_HandleTypeDef *huac)
{
    if (huac == NULL) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Process audio buffer
  * @param  huac: UAC handle pointer
  * @retval None
  */
static void UAC_ProcessAudioBuffer(UAC_HandleTypeDef *huac)
{
    if (huac->audio_data_callback != NULL) {
        huac->audio_data_callback(huac->audio_buffer, huac->buffer_index);
    }

    /* Send audio data via USB */
    UAC_SendAudioData(huac);

    /* Reset buffer */
    huac->buffer_index = 0;
}

/**
  * @brief  Send audio data via USB
  * @param  huac: UAC handle pointer
  * @retval None
  */
static void UAC_SendAudioData(UAC_HandleTypeDef *huac)
{
    if (huac->pdev == NULL || !huac->is_streaming) {
        return;
    }

    /* Send audio data via USB Audio Class */
    USBD_LL_Transmit(huac->pdev, AUDIO_OUT_EP, (uint8_t*)huac->audio_buffer, huac->buffer_index * 2);
}

/* Sample rate functions removed - not currently used */

/**
  * @brief  Generate test audio signal (sine wave)
  * @param  huac: UAC handle pointer
  * @param  frequency: Test frequency in Hz
  * @param  amplitude: Test amplitude (0-32767)
  * @param  duration_ms: Test duration in milliseconds (0 = continuous)
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_GenerateTestSignal(UAC_HandleTypeDef *huac, uint32_t frequency, uint16_t amplitude, uint32_t duration_ms)
{
    if (huac == NULL) {
        return HAL_ERROR;
    }

    uac_test_frequency = frequency;
    uac_test_amplitude = amplitude;
    uac_test_duration = duration_ms;
    uac_test_start_time = HAL_GetTick();
    uac_test_phase = 0;
    uac_test_active = 1;

    return HAL_OK;
}

/**
  * @brief  Generate test audio data for testing
  * @param  huac: UAC handle pointer
  * @param  test_mode: Test mode (0=silence, 1=sine wave, 2=square wave, 3=noise)
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_StartAudioTest(UAC_HandleTypeDef *huac, uint8_t test_mode)
{
    if (huac == NULL) {
        return HAL_ERROR;
    }

    uac_test_mode = test_mode;
    uac_test_phase = 0;
    uac_test_active = 1;
    uac_test_start_time = HAL_GetTick();

    return HAL_OK;
}

/**
  * @brief  Stop audio test
  * @param  huac: UAC handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_StopAudioTest(UAC_HandleTypeDef *huac)
{
    if (huac == NULL) {
        return HAL_ERROR;
    }

    uac_test_active = 0;
    uac_test_mode = 0;

    return HAL_OK;
}

/**
  * @brief  Process audio test data (call in main loop)
  * @param  huac: UAC handle pointer
  * @retval HAL status
  */
HAL_StatusTypeDef UAC_ProcessAudioTest(UAC_HandleTypeDef *huac)
{
    if (huac == NULL || !uac_test_active) {
        return HAL_ERROR;
    }

    /* Check duration limit */
    if (uac_test_duration > 0) {
        if ((HAL_GetTick() - uac_test_start_time) >= uac_test_duration) {
            UAC_StopAudioTest(huac);
            return HAL_OK;
        }
    }

    /* Generate test audio data */
    switch (uac_test_mode) {
        case 0: /* Silence */
            UAC_GenerateSilence(huac);
            break;
        case 1: /* Sine wave */
            UAC_GenerateSineWave(huac, uac_test_frequency, uac_test_amplitude);
            break;
        case 2: /* Square wave */
            UAC_GenerateSquareWave(huac, uac_test_frequency, uac_test_amplitude);
            break;
        case 3: /* Noise */
            UAC_GenerateNoise(huac, uac_test_amplitude);
            break;
        default:
            UAC_GenerateSilence(huac);
            break;
    }

    /* Process the generated audio data */
    return UAC_ProcessAudioData(huac, huac->audio_buffer, UAC_AUDIO_BUFFER_SIZE);
}

/* Private functions for audio generation ------------------------------------*/


/**
  * @brief  Generate sine wave test sound
  * @param  huac: UAC handle pointer
  * @param  frequency: Frequency in Hz
  * @param  amplitude: Amplitude (0-100)
  * @retval None
  */
void UAC_GenerateSineWave(UAC_HandleTypeDef *huac, uint16_t frequency, uint8_t amplitude)
{
    if (huac == NULL) return;
    
    static uint32_t sample_count = 0;
    uint16_t samples = UAC_AUDIO_BUFFER_SIZE / 2; // 16-bit samples
    int16_t max_amplitude = (int16_t)((amplitude * 32767) / 100);
    
    for (uint16_t i = 0; i < samples; i++) {
        float angle = 2.0f * 3.14159f * frequency * sample_count / 48000; // 48kHz sample rate
        int16_t sample = (int16_t)(max_amplitude * sinf(angle));
        
        huac->audio_buffer[i * 2] = sample & 0xFF;         // Low byte
        huac->audio_buffer[i * 2 + 1] = (sample >> 8) & 0xFF; // High byte
        
        sample_count++;
    }
    
    huac->buffer_index = samples;
}

/**
  * @brief  Generate square wave test sound
  * @param  huac: UAC handle pointer
  * @param  frequency: Frequency in Hz
  * @param  amplitude: Amplitude (0-100)
  * @retval None
  */
void UAC_GenerateSquareWave(UAC_HandleTypeDef *huac, uint16_t frequency, uint8_t amplitude)
{
    if (huac == NULL) return;
    
    static uint32_t sample_count = 0;
    uint16_t samples = UAC_AUDIO_BUFFER_SIZE / 2; // 16-bit samples
    int16_t max_amplitude = (int16_t)((amplitude * 32767) / 100);
    uint32_t period = 48000 / frequency; // 48kHz sample rate
    
    for (uint16_t i = 0; i < samples; i++) {
        int16_t sample = (sample_count % period < period / 2) ? max_amplitude : -max_amplitude;
        
        huac->audio_buffer[i * 2] = sample & 0xFF;         // Low byte
        huac->audio_buffer[i * 2 + 1] = (sample >> 8) & 0xFF; // High byte
        
        sample_count++;
    }
    
    huac->buffer_index = samples;
}

/**
  * @brief  Generate noise test sound
  * @param  huac: UAC handle pointer
  * @param  amplitude: Amplitude (0-100)
  * @retval None
  */
void UAC_GenerateNoise(UAC_HandleTypeDef *huac, uint8_t amplitude)
{
    if (huac == NULL) return;
    
    uint16_t samples = UAC_AUDIO_BUFFER_SIZE / 2; // 16-bit samples
    int16_t max_amplitude = (int16_t)((amplitude * 32767) / 100);
    
    for (uint16_t i = 0; i < samples; i++) {
        // Simple pseudo-random number generator
        static uint32_t seed = 1;
        seed = seed * 1103515245 + 12345;
        int16_t sample = (int16_t)((seed % (2 * max_amplitude + 1)) - max_amplitude);
        
        huac->audio_buffer[i * 2] = sample & 0xFF;         // Low byte
        huac->audio_buffer[i * 2 + 1] = (sample >> 8) & 0xFF; // High byte
    }
    
    huac->buffer_index = samples;
}

/**
  * @brief  Generate silence
  * @param  huac: UAC handle pointer
  * @retval None
  */
void UAC_GenerateSilence(UAC_HandleTypeDef *huac)
{
    if (huac == NULL) return;
    
    // Fill buffer with silence (zeros)
    for (uint16_t i = 0; i < UAC_AUDIO_BUFFER_SIZE; i++) {
        huac->audio_buffer[i] = 0;
    }
    huac->buffer_index = UAC_AUDIO_BUFFER_SIZE / 2; // 16-bit samples
}

/**
  * @brief  Test sound sequence (different sounds in sequence)
  * @param  huac: UAC handle pointer
  * @retval None
  */
void UAC_TestSoundSequence(UAC_HandleTypeDef *huac)
{
    if (huac == NULL) return;
    
    static uint32_t sequence_timer = 0;
    static uint8_t current_sound = 0;
    static uint8_t sound_sequence[] = {0, 1, 2, 3}; // silence, sine, square, noise
    static uint16_t frequencies[] = {440, 880, 1320, 1760}; // A4, A5, E6, A6
    static uint8_t amplitudes[] = {50, 70, 60, 80};
    
    // Change sound every 2 seconds
    if (HAL_GetTick() - sequence_timer > 2000) {
        current_sound = (current_sound + 1) % 4;
        sequence_timer = HAL_GetTick();
    }
    
    switch (sound_sequence[current_sound]) {
        case 0: // Silence
            UAC_GenerateSilence(huac);
            break;
        case 1: // Sine wave
            UAC_GenerateSineWave(huac, frequencies[current_sound], amplitudes[current_sound]);
            break;
        case 2: // Square wave
            UAC_GenerateSquareWave(huac, frequencies[current_sound], amplitudes[current_sound]);
            break;
        case 3: // Noise
            UAC_GenerateNoise(huac, amplitudes[current_sound]);
            break;
    }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
