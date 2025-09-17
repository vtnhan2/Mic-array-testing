# Hướng dẫn tích hợp UAC Driver vào STM32

**Author:** Nhan Vo  
**Date:** 2025-09-14  
**Version:** 1.0

## Tổng quan

Driver UAC đã được tích hợp thành công vào project STM32 để tạo microphone USB. Project hiện tại bao gồm:

- ✅ **UAC.h** - Header file với API đầy đủ
- ✅ **UAC.c** - Implementation hoàn chỉnh
- ✅ **main.c** - Đã tích hợp UAC driver
- ✅ **Audio Test Functions** - Các function test audio

## Cấu trúc tích hợp

### 1. Files đã được thêm vào project:

```
Core/
├── Inc/
│   ├── UAC.h              # UAC driver header
│   └── UAC_Example.h      # Example functions
└── Src/
    ├── UAC.c              # UAC driver implementation
    ├── UAC_Example.c      # Example implementation
    └── main.c             # Đã tích hợp UAC
```

### 2. Thay đổi trong main.c:

#### Includes:
```c
#include "UAC.h"
#include <math.h>
#include <stdlib.h>
```

#### Variables:
```c
UAC_HandleTypeDef huac;
UAC_AudioConfig_t audio_config;
uint8_t uac_initialized = 0;
uint8_t test_mode = 1; /* 0=silence, 1=sine wave, 2=square wave, 3=noise */
```

#### Functions:
```c
void UAC_Init_Microphone(void);
void UAC_Test_Audio(void);
```

## Cách sử dụng

### 1. Khởi tạo tự động

UAC microphone sẽ được khởi tạo tự động trong `main()` function:

```c
int main(void)
{
  // ... system initialization ...
  
  /* Initialize UAC Microphone */
  UAC_Init_Microphone();
  
  while (1)
  {
    /* Process UAC Audio Test */
    if (uac_initialized) {
      UAC_Test_Audio();
    }
    
    HAL_Delay(1);
  }
}
```

### 2. Audio Test Modes

Driver tự động chuyển đổi giữa các test mode mỗi 5 giây:

- **Mode 0**: Silence (im lặng)
- **Mode 1**: Sine wave 1kHz (sóng sin 1kHz)
- **Mode 2**: Square wave 2kHz (sóng vuông 2kHz)  
- **Mode 3**: Noise (nhiễu trắng)

### 3. UART Debug Output

Kết nối UART2 (115200 baud) để xem debug messages:

```
UAC Microphone initialized successfully!
Test mode changed
Test mode changed
...
```

## API Functions

### Khởi tạo và cấu hình:

```c
// Khởi tạo UAC device
HAL_StatusTypeDef UAC_Init(UAC_HandleTypeDef *huac, USBD_HandleTypeDef *pdev, UAC_AudioConfig_t *config);

// Bắt đầu streaming
HAL_StatusTypeDef UAC_StartStreaming(UAC_HandleTypeDef *huac);

// Dừng streaming
HAL_StatusTypeDef UAC_StopStreaming(UAC_HandleTypeDef *huac);
```

### Audio Test Functions:

```c
// Bắt đầu audio test
HAL_StatusTypeDef UAC_StartAudioTest(UAC_HandleTypeDef *huac, uint8_t test_mode);

// Tạo test signal cụ thể
HAL_StatusTypeDef UAC_GenerateTestSignal(UAC_HandleTypeDef *huac, uint32_t frequency, uint16_t amplitude, uint32_t duration_ms);

// Dừng audio test
HAL_StatusTypeDef UAC_StopAudioTest(UAC_HandleTypeDef *huac);

// Xử lý audio test (gọi trong main loop)
HAL_StatusTypeDef UAC_ProcessAudioTest(UAC_HandleTypeDef *huac);
```

### Xử lý audio data:

```c
// Xử lý audio data từ I2S
HAL_StatusTypeDef UAC_ProcessAudioData(UAC_HandleTypeDef *huac, uint16_t *data, uint16_t length);

// Thiết lập callback function
HAL_StatusTypeDef UAC_SetAudioDataCallback(UAC_HandleTypeDef *huac, void (*callback)(uint16_t *data, uint16_t length));
```

## Cấu hình Audio

### Audio Configuration:

```c
UAC_AudioConfig_t audio_config = {
    .format = UAC_FORMAT_PCM,           // PCM format
    .sample_rate = UAC_SAMPLE_RATE_48K, // 48kHz sample rate
    .channels = 1,                      // Mono microphone
    .bits_per_sample = 16,              // 16-bit audio
    .frame_size = 2                     // 2 bytes per sample
};
```

### Sample Rates hỗ trợ:

- `UAC_SAMPLE_RATE_8K` = 8000 Hz
- `UAC_SAMPLE_RATE_16K` = 16000 Hz
- `UAC_SAMPLE_RATE_22K` = 22050 Hz
- `UAC_SAMPLE_RATE_44K` = 44100 Hz
- `UAC_SAMPLE_RATE_48K` = 48000 Hz
- `UAC_SAMPLE_RATE_96K` = 96000 Hz

## Test và Debug

### 1. UART Debug

Kết nối UART2 để xem status messages:
- Baud rate: 115200
- Data bits: 8
- Stop bits: 1
- Parity: None

### 2. Audio Test

Driver tự động tạo test audio signals:
- **Sine wave**: Tần số có thể điều chỉnh (1kHz, 2kHz, etc.)
- **Square wave**: Tần số có thể điều chỉnh
- **Noise**: Nhiễu trắng với amplitude có thể điều chỉnh
- **Silence**: Im lặng hoàn toàn

### 3. USB Connection

Khi kết nối USB, STM32 sẽ xuất hiện như một microphone device:
- Device name: "STM32 UAC Microphone"
- Sample rate: 48kHz
- Format: 16-bit PCM Mono

## Tùy chỉnh

### 1. Thay đổi test mode:

```c
// Trong main.c, thay đổi test_mode
uint8_t test_mode = 2; // 0=silence, 1=sine, 2=square, 3=noise
```

### 2. Thay đổi tần số test:

```c
// Trong UAC_Test_Audio(), thay đổi frequency
UAC_GenerateTestSignal(&huac, 5000, 16000, 0); // 5kHz sine wave
```

### 3. Thay đổi amplitude:

```c
// Amplitude từ 0-32767 (0 = silence, 32767 = max)
UAC_GenerateTestSignal(&huac, 1000, 8000, 0); // 1kHz, 25% amplitude
```

## Lưu ý quan trọng

1. **USB OTG FS**: Cần có USB OTG FS đã được cấu hình
2. **I2S Interface**: Có thể sử dụng I2S để nhận audio data thực
3. **Real-time processing**: Audio test chạy trong main loop
4. **Buffer management**: Sử dụng buffer 256 samples (16-bit)
5. **Math library**: Cần link với math library cho sin() function

## Troubleshooting

### Lỗi thường gặp:

1. **USB không nhận diện**: Kiểm tra USB descriptor và cấu hình
2. **Không có âm thanh**: Kiểm tra test mode và amplitude
3. **Chất lượng âm thanh kém**: Kiểm tra sample rate và bit depth
4. **Compile error**: Kiểm tra math library linking

### Debug steps:

1. Kiểm tra UART output
2. Kiểm tra USB connection
3. Kiểm tra audio test mode
4. Kiểm tra buffer data

## Kết luận

Driver UAC đã được tích hợp thành công vào STM32 project. STM32 hiện có thể hoạt động như một microphone USB với các tính năng test audio đầy đủ. Driver hoàn toàn độc lập và không can thiệp vào code có sẵn.
