# USB Audio Class (UAC) Driver for STM32

**Author:** Nhan Vo  
**Date:** 2025-09-14  
**Version:** 1.0

## Tổng quan

Driver UAC này cho phép STM32 hoạt động như một microphone USB (input device) cho laptop. Driver được thiết kế như một thư viện độc lập, có thể tái sử dụng mà không cần thay đổi các file khác trong project.

## Tính năng

- ✅ Hỗ trợ USB Audio Class 1.0 specification
- ✅ Microphone input (mono/stereo)
- ✅ Nhiều sample rate: 8kHz, 16kHz, 22kHz, 44kHz, 48kHz, 96kHz
- ✅ Hỗ trợ PCM format 16-bit
- ✅ Callback function cho xử lý audio data
- ✅ Tương thích với STM32F4xx series
- ✅ Driver độc lập, không can thiệp vào code có sẵn

## Cấu trúc file

```
Core/
├── Inc/
│   ├── UAC.h              # Header file chính
│   └── UAC_Example.h      # File example và hướng dẫn sử dụng
└── Src/
    ├── UAC.c              # Implementation chính
    └── UAC_Example.c      # Code example
```

## Cách sử dụng

### 1. Khởi tạo UAC device

```c
#include "UAC.h"
#include "UAC_Example.h"

// Khai báo handle
UAC_HandleTypeDef huac;
UAC_AudioConfig_t audio_config;

// Cấu hình audio
audio_config.format = UAC_FORMAT_PCM;
audio_config.sample_rate = UAC_SAMPLE_RATE_48K;
audio_config.channels = 1;  // Mono microphone
audio_config.bits_per_sample = 16;
audio_config.frame_size = 2;

// Khởi tạo UAC
HAL_StatusTypeDef status = UAC_Init(&huac, &hpcd_USB_OTG_FS, &audio_config);
```

### 2. Thiết lập callback function

```c
// Định nghĩa callback function
void MyAudioDataCallback(uint16_t *data, uint16_t length)
{
    // Xử lý audio data nếu cần
    // Data sẽ được gửi tự động qua USB
}

// Thiết lập callback
UAC_SetAudioDataCallback(&huac, MyAudioDataCallback);
```

### 3. Bắt đầu streaming

```c
// Bắt đầu audio streaming
UAC_StartStreaming(&huac);
```

### 4. Xử lý audio data từ I2S

```c
// Trong main loop hoặc interrupt
uint16_t audio_data[UAC_AUDIO_BUFFER_SIZE];

// Nhận data từ I2S
HAL_I2S_Receive(&hi2s1, (uint16_t*)audio_data, UAC_AUDIO_BUFFER_SIZE, HAL_MAX_DELAY);

// Gửi data qua UAC
UAC_ProcessAudioData(&huac, audio_data, UAC_AUDIO_BUFFER_SIZE);
```

## Cấu hình USB

Để sử dụng driver này, project cần có:

1. **USB OTG FS** đã được cấu hình
2. **I2S** interface cho audio input
3. **USB descriptors** phù hợp

### USB Descriptor cần thiết

Driver tự động cung cấp các descriptor cần thiết:
- Device Descriptor
- Configuration Descriptor  
- Interface Descriptor (Audio Control + Audio Streaming)
- Endpoint Descriptor (Audio Data IN)

## API Reference

### Hàm khởi tạo

```c
HAL_StatusTypeDef UAC_Init(UAC_HandleTypeDef *huac, USBD_HandleTypeDef *pdev, UAC_AudioConfig_t *config);
HAL_StatusTypeDef UAC_DeInit(UAC_HandleTypeDef *huac);
```

### Hàm điều khiển streaming

```c
HAL_StatusTypeDef UAC_StartStreaming(UAC_HandleTypeDef *huac);
HAL_StatusTypeDef UAC_StopStreaming(UAC_HandleTypeDef *huac);
```

### Hàm xử lý audio data

```c
HAL_StatusTypeDef UAC_ProcessAudioData(UAC_HandleTypeDef *huac, uint16_t *data, uint16_t length);
HAL_StatusTypeDef UAC_SetAudioDataCallback(UAC_HandleTypeDef *huac, void (*callback)(uint16_t *data, uint16_t length));
```

### Hàm cấu hình audio

```c
HAL_StatusTypeDef UAC_SetAudioConfig(UAC_HandleTypeDef *huac, UAC_AudioConfig_t *config);
HAL_StatusTypeDef UAC_GetAudioConfig(UAC_HandleTypeDef *huac, UAC_AudioConfig_t *config);
```

## Cấu hình Audio

### Sample Rate hỗ trợ

```c
typedef enum {
    UAC_SAMPLE_RATE_8K = 8000,
    UAC_SAMPLE_RATE_16K = 16000,
    UAC_SAMPLE_RATE_22K = 22050,
    UAC_SAMPLE_RATE_44K = 44100,
    UAC_SAMPLE_RATE_48K = 48000,
    UAC_SAMPLE_RATE_96K = 96000
} UAC_SampleRate_t;
```

### Audio Format hỗ trợ

```c
typedef enum {
    UAC_FORMAT_PCM = 0x01,
    UAC_FORMAT_PCM8 = 0x02,
    UAC_FORMAT_IEEE_FLOAT = 0x03,
    UAC_FORMAT_ALAW = 0x04,
    UAC_FORMAT_MULAW = 0x05
} UAC_AudioFormat_t;
```

## Ví dụ hoàn chỉnh

Xem file `UAC_Example.c` để có ví dụ hoàn chỉnh về cách sử dụng driver.

## Lưu ý quan trọng

1. **Không thay đổi file khác**: Driver được thiết kế để hoạt động độc lập
2. **USB OTG FS**: Cần có USB OTG FS đã được cấu hình sẵn
3. **I2S Interface**: Cần có I2S interface để nhận audio data
4. **Buffer size**: Mặc định sử dụng buffer 256 samples (16-bit)
5. **Real-time processing**: Cần xử lý audio data trong thời gian thực

## Troubleshooting

### Lỗi thường gặp

1. **USB không nhận diện**: Kiểm tra USB descriptor và cấu hình USB OTG
2. **Không có âm thanh**: Kiểm tra I2S configuration và audio data flow
3. **Chất lượng âm thanh kém**: Kiểm tra sample rate và bit depth
4. **Buffer overflow**: Tăng buffer size hoặc tối ưu xử lý audio

### Debug

Sử dụng UART để debug:
```c
printf("UAC Status: %d\n", huac.is_streaming);
printf("Buffer Index: %d\n", huac.buffer_index);
```

## License

Copyright (c) 2025 STMicroelectronics. All rights reserved.

## Hỗ trợ

Để được hỗ trợ, vui lòng liên hệ STMicroelectronics hoặc tham khảo tài liệu STM32 USB Audio Class.
