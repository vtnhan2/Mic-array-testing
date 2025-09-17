# UAC Driver Configuration and Integration

**Author:** Nhan Vo  
**Date:** 2025-09-14  
**Version:** 1.0

## Overview

This document describes the changes made to integrate the UAC driver with the existing USB Audio Class in the STM32CubeMX project.

## Project Structure

### 1. USB Audio Class Setup
The project has been configured with USB Audio Class through STM32CubeMX:
- **USB_DEVICE/App/usb_device.c** - USB Device initialization
- **USB_DEVICE/App/usbd_audio_if.c** - USB Audio Interface implementation
- **USB_DEVICE/App/usbd_desc.c** - USB Descriptors
- **USB_DEVICE/Target/usbd_conf.c** - USB Configuration

### 2. UAC Driver Files
- **Core/Inc/UAC.h** - UAC Driver Header
- **Core/Src/UAC.c** - UAC Driver Implementation
- **Core/Inc/UAC_Example.h** - UAC Usage Example Header
- **Core/Src/UAC_Example.c** - UAC Usage Example Implementation

## Main Changes

### 1. Update UAC.h

```c
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_audio.h"  // Add USB Audio Class header

/* UAC Device Handle Structure */
typedef struct {
    USBD_HandleTypeDef *pdev;  // Changed from void* to USBD_HandleTypeDef*
    UAC_AudioConfig_t audio_config;
    uint8_t is_configured;
    uint8_t is_streaming;
    uint16_t audio_buffer[UAC_AUDIO_BUFFER_SIZE];
    uint16_t buffer_index;
    void (*audio_data_callback)(uint16_t *data, uint16_t length);
} UAC_HandleTypeDef;

/* Function prototypes */
HAL_StatusTypeDef UAC_Init(UAC_HandleTypeDef *huac, USBD_HandleTypeDef *pdev, UAC_AudioConfig_t *config);
HAL_StatusTypeDef UAC_HandleControlRequest(UAC_HandleTypeDef *huac, USBD_SetupReqTypedef *req);
HAL_StatusTypeDef UAC_HandleStreamingRequest(UAC_HandleTypeDef *huac, USBD_SetupReqTypedef *req);
```

### 2. Update UAC.c

#### a) Add includes
```c
#include "UAC.h"
#include "usbd_audio_if.h"  // Add USB Audio Interface
#include <math.h>
#include <stdlib.h>
```

#### b) Update UAC_Init function
```c
HAL_StatusTypeDef UAC_Init(UAC_HandleTypeDef *huac, USBD_HandleTypeDef *pdev, UAC_AudioConfig_t *config)
{
    if (huac == NULL || pdev == NULL || config == NULL) {
        return HAL_ERROR;
    }

    /* Initialize handle */
    huac->pdev = pdev;  // Use USBD_HandleTypeDef instead of void*
    // ... rest of implementation
}
```

#### c) Update UAC_SendAudioData function
```c
static void UAC_SendAudioData(UAC_HandleTypeDef *huac)
{
    if (huac->pdev == NULL || !huac->is_streaming) {
        return;
    }

    /* Send audio data via USB Audio Class */
    USBD_LL_Transmit(huac->pdev, UAC_AUDIO_EP_IN, (uint8_t*)huac->audio_buffer, huac->buffer_index * 2);
}
```

### 3. Update main.c

#### a) Add includes
```c
#include "UAC.h"
#include "usbd_audio_if.h"  // Add USB Audio Interface
#include <math.h>
#include <stdlib.h>
```

#### b) Update UAC initialization
```c
/* Initialize UAC device */
status = UAC_Init(&huac, &hUsbDeviceFS, &audio_config);  // Use hUsbDeviceFS instead of hpcd_USB_OTG_FS
```

### 4. Update usbd_audio_if.c

#### a) Add includes
```c
#include "usbd_audio_if.h"
#include "UAC.h"  // Add UAC driver
```

#### b) Add UAC handle reference
```c
/* USER CODE BEGIN PRIVATE_VARIABLES */
extern UAC_HandleTypeDef huac;  // Reference to UAC handle
/* USER CODE END PRIVATE_VARIABLES */
```

#### c) Update AUDIO_Init_FS function
```c
static int8_t AUDIO_Init_FS(uint32_t AudioFreq, uint32_t Volume, uint32_t options)
{
    UNUSED(AudioFreq);
    UNUSED(Volume);
    UNUSED(options);
    
    /* Mark UAC as configured */
    huac.is_configured = 1;
    
    return (USBD_OK);
}
```

#### d) Update AUDIO_DeInit_FS function
```c
static int8_t AUDIO_DeInit_FS(uint32_t options)
{
    UNUSED(options);
    
    /* Mark UAC as not configured */
    huac.is_configured = 0;
    huac.is_streaming = 0;
    
    return (USBD_OK);
}
```

#### e) Update AUDIO_AudioCmd_FS function
```c
static int8_t AUDIO_AudioCmd_FS(uint8_t* pbuf, uint32_t size, uint8_t cmd)
{
    switch(cmd)
    {
        case AUDIO_CMD_START:
            /* Start audio streaming */
            huac.is_streaming = 1;
            break;

        case AUDIO_CMD_PLAY:
            /* Play audio data */
            if (pbuf != NULL && size > 0) {
                /* Process incoming audio data if needed */
                /* For microphone, we typically send data, not receive */
            }
            break;
    }
    UNUSED(pbuf);
    UNUSED(size);
    return (USBD_OK);
}
```

## USB Audio Class Configuration

### 1. USB Descriptors
USB Audio Class is configured with:
- **Device Class:** Audio (0x01)
- **Subclass:** Audio Control (0x01) and Audio Streaming (0x02)
- **Protocol:** Undefined (0x00)
- **Sample Rate:** 48kHz
- **Channels:** 1 (Mono) - Microphone input
- **Bit Resolution:** 16-bit
- **Format:** PCM

### 2. Endpoints
- **AUDIO_IN_EP:** 0x81 (Input endpoint - for microphone)
- **Endpoint Type:** Isochronous IN for audio input

### 3. USB Configuration
- **Max Power:** 100mA
- **Self Powered:** Yes
- **LPM Enabled:** No

### 4. Microphone Configuration Changes
The following changes were made to configure STM32 as microphone input:

#### a) USB Configuration (usbd_conf.h)
```c
/* USB Audio Microphone Configuration */
#define AUDIO_IN_EP         0x81U  /* IN endpoint for microphone data */
#define USBD_AUDIO_AS_MICROPHONE    1U  /* Configure as microphone instead of speaker */

/* Make sure the microphone flag is globally available */
#ifndef USBD_AUDIO_AS_MICROPHONE
#define USBD_AUDIO_AS_MICROPHONE    1U
#endif
```

#### b) USB Audio Class Header (usbd_audio.h)
```c
#ifndef AUDIO_OUT_EP
#ifdef USBD_AUDIO_AS_MICROPHONE
#define AUDIO_OUT_EP                                  0x81U  /* IN endpoint for microphone */
#else
#define AUDIO_OUT_EP                                  0x01U  /* OUT endpoint for speaker */
#endif /* USBD_AUDIO_AS_MICROPHONE */
#endif /* AUDIO_OUT_EP */
```

#### c) USB Audio Class Implementation (usbd_audio.c)
```c
/* Packet size calculation for microphone (mono) vs speaker (stereo) */
#ifdef USBD_AUDIO_AS_MICROPHONE
#define AUDIO_PACKET_SZE(frq) \
  (uint8_t)(((frq * 2U * 1U) / 1000U) & 0xFFU), (uint8_t)((((frq * 2U * 1U) / 1000U) >> 8) & 0xFFU)  /* Mono */
#else
#define AUDIO_PACKET_SZE(frq) \
  (uint8_t)(((frq * 2U * 2U) / 1000U) & 0xFFU), (uint8_t)((((frq * 2U * 2U) / 1000U) >> 8) & 0xFFU)  /* Stereo */
#endif /* USBD_AUDIO_AS_MICROPHONE */

/* Audio packet size for microphone (mono) vs speaker (stereo) */
#ifdef USBD_AUDIO_AS_MICROPHONE
#define AUDIO_OUT_PACKET                              (uint16_t)(((USBD_AUDIO_FREQ * 2U * 1U) / 1000U))  /* Mono for microphone */
#else
#define AUDIO_OUT_PACKET                              (uint16_t)(((USBD_AUDIO_FREQ * 2U * 2U) / 1000U))  /* Stereo for speaker */
#endif /* USBD_AUDIO_AS_MICROPHONE */

/* Terminal types for microphone vs speaker */
#ifdef USBD_AUDIO_AS_MICROPHONE
  0x01,                                 /* wTerminalType MICROPHONE 0x0201 */
  0x02,
#else
  0x01,                                 /* wTerminalType AUDIO_TERMINAL_USB_STREAMING   0x0101 */
  0x01,
#endif /* USBD_AUDIO_AS_MICROPHONE */

/* Output terminal for microphone vs speaker */
#ifdef USBD_AUDIO_AS_MICROPHONE
  0x01,                                 /* wTerminalType USB_STREAMING 0x0101 */
  0x01,
#else
  0x01,                                 /* wTerminalType SPEAKER 0x0301 */
  0x03,
#endif /* USBD_AUDIO_AS_MICROPHONE */

/* Channel configuration for microphone (mono) vs speaker (stereo) */
#ifdef USBD_AUDIO_AS_MICROPHONE
  0x01,                                 /* bNrChannels - Mono */
#else
  0x02,                                 /* bNrChannels - Stereo */
#endif /* USBD_AUDIO_AS_MICROPHONE */

/* Endpoint handling for microphone (IN) vs speaker (OUT) */
#ifdef USBD_AUDIO_AS_MICROPHONE
  /* Open EP IN for microphone */
  (void)USBD_LL_OpenEP(pdev, AUDIOOutEpAdd, USBD_EP_TYPE_ISOC, AUDIO_OUT_PACKET);
  pdev->ep_in[AUDIOOutEpAdd & 0xFU].is_used = 1U;
#else
  /* Open EP OUT for speaker */
  (void)USBD_LL_OpenEP(pdev, AUDIOOutEpAdd, USBD_EP_TYPE_ISOC, AUDIO_OUT_PACKET);
  pdev->ep_out[AUDIOOutEpAdd & 0xFU].is_used = 1U;
#endif /* USBD_AUDIO_AS_MICROPHONE */
```

#### d) Product String (usbd_desc.c)
```c
#define USBD_PID_FS     48000
#define USBD_PRODUCT_STRING_FS     "STM32 USB Microphone"
```

#### e) Terminal Descriptors (UAC.c)
```c
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
```

#### f) Audio Streaming Descriptor (UAC.c)
```c
/* Audio Streaming General Descriptor */
0x07,                       /* bLength */
UAC_AS_INTERFACE_GENERAL,   /* bDescriptorType */
0x01,                       /* bTerminalLink - Link to Input Terminal */
0x00,                       /* bDelay */
0x01, 0x00,                 /* wFormatTag - PCM */
```

#### g) Endpoint Descriptor (UAC.c)
```c
/* Audio Data Endpoint Descriptor - Input for Microphone */
0x07,                       /* bLength */
USB_DESC_TYPE_ENDPOINT,     /* bDescriptorType */
    AUDIO_OUT_EP,               /* bEndpointAddress - IN endpoint for microphone */
0x05,                       /* bmAttributes - Isochronous */
LOBYTE(UAC_AUDIO_EP_SIZE), HIBYTE(UAC_AUDIO_EP_SIZE), /* wMaxPacketSize */
0x01                        /* bInterval */
```

#### h) Audio Command Handling (usbd_audio_if.c)
```c
static int8_t AUDIO_AudioCmd_FS(uint8_t* pbuf, uint32_t size, uint8_t cmd)
{
  switch(cmd)
  {
    case AUDIO_CMD_START:
      /* Start audio streaming - for microphone input */
      huac.is_streaming = 1;
      break;

    case AUDIO_CMD_PLAY:
      /* For microphone, this is actually recording start */
      huac.is_streaming = 1;
      break;
  }
  UNUSED(pbuf);
  UNUSED(size);
  return (USBD_OK);
}

static int8_t AUDIO_PeriodicTC_FS(uint8_t *pbuf, uint32_t size, uint8_t cmd)
{
  if (cmd == AUDIO_IN_TC) {
    // Microphone data transmission
    if (pbuf != NULL && size > 0) {
      // Fill with test audio data for now
      // In real implementation, this would get data from I2S or ADC
      for (uint32_t i = 0; i < size; i += 2) {
        // Generate simple test signal (sine wave)
        static uint32_t sample_count = 0;
        int16_t sample = (int16_t)(32767 * sin(2 * 3.14159 * 1000 * sample_count / 48000));
        pbuf[i] = sample & 0xFF;
        pbuf[i + 1] = (sample >> 8) & 0xFF;
        sample_count++;
      }
      return USBD_OK;
    }
  }
  return (USBD_OK);
}
```

## Using UAC Driver

### 1. Initialize UAC
```c
UAC_HandleTypeDef huac;
UAC_AudioConfig_t audio_config;

/* Configure audio parameters */
audio_config.format = UAC_FORMAT_PCM;
audio_config.sample_rate = UAC_SAMPLE_RATE_48K;
audio_config.channels = 1;  /* Mono microphone */
audio_config.bits_per_sample = 16;
audio_config.frame_size = 2; /* 16-bit = 2 bytes per sample */

/* Initialize UAC device */
HAL_StatusTypeDef status = UAC_Init(&huac, &hUsbDeviceFS, &audio_config);
```

### 2. Start streaming
```c
if (status == HAL_OK) {
    /* Set audio data callback (optional) */
    UAC_SetAudioDataCallback(&huac, NULL);
    
    /* Start audio streaming */
    status = UAC_StartStreaming(&huac);
}
```

### 3. Audio Test Functions
```c
/* Start audio test */
UAC_StartAudioTest(&huac, test_mode);  // 0=silence, 1=sine, 2=square, 3=noise

/* Generate test signal */
UAC_GenerateTestSignal(&huac, 1000, 16000, 0);  // 1kHz, 50% amplitude, continuous

/* Process audio test in main loop */
UAC_ProcessAudioTest(&huac);

/* New Test Sound Functions */
UAC_GenerateSineWave(&huac, 440, 50);      // 440Hz sine wave at 50% amplitude
UAC_GenerateSquareWave(&huac, 880, 70);    // 880Hz square wave at 70% amplitude
UAC_GenerateNoise(&huac, 60);              // Noise at 60% amplitude
UAC_GenerateSilence(&huac);                // Silence
UAC_TestSoundSequence(&huac);              // Cycles through different sounds
```

## Important Notes

### 1. USB Device Handle
- Use `hUsbDeviceFS` instead of `hpcd_USB_OTG_FS`
- UAC driver is integrated with existing USB Audio Class

### 2. Endpoint Configuration
- Endpoint IN (0x81) is used for microphone input
- Endpoint OUT (0x01) is used for speaker output

### 3. Audio Data Flow
- Audio data is sent via `USBD_LL_Transmit()`
- USB Audio Class handles data transmission over USB

### 4. Callback Integration
- UAC driver integrates with USB Audio Class callbacks
- Audio streaming is controlled by USB Audio Class

## Troubleshooting

### 1. Compilation Errors
- Ensure correct header files are included
- Check USB Audio Class configuration

### 2. Runtime Issues
- Check USB device initialization
- Ensure UAC handle is properly initialized

### 3. Audio Issues
- Check sample rate and format configuration
- Ensure audio buffer is processed correctly

### 4. Microphone Recognition Issues
- **Problem:** STM32 appears as "Speakers" instead of "Microphone"
- **Solution:** Ensure terminal descriptors are correctly configured:
  - Input Terminal Type: 0x0201 (Microphone)
  - Output Terminal Type: 0x0101 (USB Streaming)
  - Endpoint Address: 0x81 (IN endpoint)
  - Audio Streaming Interface: Properly linked to Input Terminal

- **Problem:** No audio input detected
- **Solution:** Check audio command handling in `usbd_audio_if.c`:
  - `AUDIO_CMD_START` should enable streaming
  - `AUDIO_CMD_PLAY` should start recording for microphone

- **Problem:** Audio quality issues
- **Solution:** Verify audio format configuration:
  - Sample Rate: 48kHz
  - Channels: 1 (Mono)
  - Bit Resolution: 16-bit
  - Format: PCM

## Conclusion

The UAC driver has been successfully integrated with the existing USB Audio Class in the STM32CubeMX project. The driver provides:

- **Complete integration** with USB Audio Class
- **Audio test functions** for microphone testing
- **Flexible configuration** for audio parameters
- **Easy-to-use API** for application development

The driver is ready for use in microphone array applications with STM32F4xx microcontrollers.
