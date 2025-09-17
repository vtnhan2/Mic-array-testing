# STM32F4 Microphone Array Project

**Author:** Nhan Vo  
**Date:** September 17, 2025  
**Version:** 1.0

## Tổng quan

Project này triển khai hệ thống microphone array sử dụng STM32F411CEUx với khả năng thu âm thanh đa kênh và truyền dữ liệu qua USB Audio Class. Hệ thống hỗ trợ 4 microphone kết nối qua I2S và có thể hoạt động như USB microphone.

## Tính năng chính

- ✅ **4-channel microphone array** với I2S interface
- ✅ **USB Audio Class** - hoạt động như USB microphone  
- ✅ **Real-time audio processing** với DMA
- ✅ **Multi-channel to mono conversion**
- ✅ **Gain control** (0-100%)
- ✅ **Audio test functions** với sine wave, square wave, noise
- ✅ **Debug output** qua UART2
- ❌ **Audio direction detection** (chưa triển khai)

## Cấu trúc Project

```
mic_array_stm32/
├── Core/
│   ├── Inc/
│   │   ├── main.h              # Main header với GPIO definitions
│   │   ├── MIC_ARRAY.h         # Microphone array driver header
│   │   ├── UAC.h               # USB Audio Class driver header
│   │   └── UAC_Example.h       # Audio test functions header
│   └── Src/
│       ├── main.c              # Main application logic
│       ├── MIC_ARRAY.c         # Microphone array driver implementation
│       ├── UAC.c               # USB Audio Class driver implementation
│       └── UAC_Example.c       # Audio test functions implementation
├── USB_DEVICE/                 # USB Device configuration
├── Drivers/                    # STM32 HAL drivers
├── Middlewares/                # USB middleware
├── config_UAC.md              # UAC configuration guide
├── UAC_Integration_Guide.md    # UAC integration guide
└── mic_array_stm32.ioc        # STM32CubeMX configuration file
```

## Cấu hình phần cứng

### STM32F411CEUx Pin Configuration

#### I2S Interfaces (4 microphones)

| Microphone | I2S | Clock Pin | WS Pin | Data Pin | Mode |
|------------|-----|-----------|--------|----------|------|
| MIC_0 | I2S1 | PA5 (mic_clk_0) | PA4 (mic_ws_0) | PA7 (mic_dout_0) | Slave RX |
| MIC_1 | I2S2 | PB10 (mic_clk_1) | PB9 (mic_ws_1) | PB15 (mic_dout_1) | Master RX |
| MIC_2 | I2S4 | PB13 (mic_clk_2) | PB12 (mic_ws_2) | PA1 (mic_dout_2) | Slave RX |
| MIC_3 | I2S5 | PB0 (mic_clk_3) | PB1 (mic_ws_3) | PA10 (mic_dout_3) | Slave RX |

#### Các chân khác

| Chức năng | Pin | Mô tả |
|-----------|-----|-------|
| USB D- | PA11 | USB OTG FS Data - |
| USB D+ | PA12 | USB OTG FS Data + |
| UART TX | PA2 | Debug output |
| UART RX | PA3 | Debug input |
| LED Clock | PB3 | SPI3 SCK cho LED control |
| LED Data | PB5 | SPI3 MOSI cho LED control |
| SWDIO | PA13 | Debug interface |
| SWCLK | PA14 | Debug interface |

### Kết nối Microphone Array với STM32F4

#### Sơ đồ kết nối cơ bản

```
Microphone Array          STM32F411CEUx
┌─────────────────┐      ┌─────────────────┐
│                 │      │                 │
│ MIC0_CLK   ────────────▶ PA5 (I2S1_CK)  │
│ MIC0_WS    ────────────▶ PA4 (I2S1_WS)  │
│ MIC0_DATA  ────────────▶ PA7 (I2S1_SD)  │
│                 │      │                 │
│ MIC1_CLK   ◀──────────── PB10 (I2S2_CK) │ (Master)
│ MIC1_WS    ◀──────────── PB9 (I2S2_WS)  │ (Master)
│ MIC1_DATA  ────────────▶ PB15 (I2S2_SD) │
│                 │      │                 │
│ MIC2_CLK   ────────────▶ PB13 (I2S4_CK) │
│ MIC2_WS    ────────────▶ PB12 (I2S4_WS) │
│ MIC2_DATA  ────────────▶ PA1 (I2S4_SD)  │
│                 │      │                 │
│ MIC3_CLK   ────────────▶ PB0 (I2S5_CK)  │
│ MIC3_WS    ────────────▶ PB1 (I2S5_WS)  │
│ MIC3_DATA  ────────────▶ PA10 (I2S5_SD) │
│                 │      │                 │
│ VCC        ────────────▶ 3.3V            │
│ GND        ────────────▶ GND             │
└─────────────────┘      └─────────────────┘
```

#### Lưu ý kết nối

1. **I2S2 là Master**: Cung cấp clock cho tất cả microphones
2. **I2S1, I2S4, I2S5 là Slave**: Nhận clock từ I2S2 hoặc external source
3. **Nguồn điện**: 3.3V từ STM32
4. **Ground**: Kết nối chung GND

## Cấu hình STM32 thành Microphone

### 1. STM32CubeMX Configuration

#### Clock Configuration
- **HSE**: 25MHz external oscillator
- **PLL**: 192MHz (PLLM=25, PLLN=192, PLLP=2, PLLQ=4)
- **SYSCLK**: 96MHz
- **I2S Clock**: 150MHz

#### I2S Configuration
```c
// I2S Parameters
Audio Frequency: 48kHz
Data Format: 32-bit
Standard: Philips
CPOL: Low
Clock Source: PLL
```

#### DMA Configuration
```c
// DMA Streams for I2S RX
SPI1_RX: DMA2_Stream0 (Circular mode)
SPI2_RX: DMA1_Stream3 (Circular mode)  
SPI4_RX: DMA2_Stream3 (Circular mode)
SPI5_RX: DMA2_Stream5 (Circular mode)
```

#### USB Configuration
```c
// USB Audio Class
Device Class: Audio (0x01)
Sample Rate: 48kHz
Channels: 1 (Mono)
Bit Resolution: 16-bit
Format: PCM
Product String: "STM32 Microphone"
```

### 2. Code Configuration

#### Microphone Array Initialization

```c
void MIC_ARRAY_Init_Microphones(void)
{
    // Configure parameters
    mic_array_config.sample_rate = 48000;    // 48kHz
    mic_array_config.channels = 4;           // 4 microphones
    mic_array_config.bits_per_sample = 16;   // 16-bit
    mic_array_config.buffer_size = MIC_ARRAY_BUFFER_SIZE; // 512 samples
    mic_array_config.mic_count = 4;
    
    // Initialize mic array
    MIC_ARRAY_Init(&hmic_array, &hi2s2, &mic_array_config);
    MIC_ARRAY_StartStreaming(&hmic_array);
}
```

#### USB Audio Initialization

```c
void UAC_Init_Microphone(void)
{
    // Configure audio parameters
    audio_config.format = UAC_FORMAT_PCM;
    audio_config.sample_rate = UAC_SAMPLE_RATE_48K;
    audio_config.channels = 1;  // Mono microphone
    audio_config.bits_per_sample = 16;
    audio_config.frame_size = 2;
    
    // Initialize UAC device
    UAC_Init(&huac, &hUsbDeviceFS, &audio_config);
    UAC_StartStreaming(&huac);
}
```

## Vấn đề cần xử lý

### 1. Vấn đề hiện tại

#### ❌ Audio Direction Detection chưa được triển khai
- **Vấn đề**: Code hiện tại chỉ convert multi-channel sang mono bằng cách average
- **Giải pháp cần triển khai**: 
  - Beamforming algorithms
  - Time Difference of Arrival (TDOA)
  - Cross-correlation analysis
  - Direction of Arrival (DOA) estimation

#### ❌ GPIO Configuration không hoàn chỉnh
- **Vấn đề**: `MIC_ARRAY_GPIO_Init()` function rỗng
- **Giải pháp**: GPIO đã được config trong STM32CubeMX, function này chỉ để compatibility

#### ❌ I2S Synchronization
- **Vấn đề**: Chỉ có I2S2 là master, các I2S khác cần sync
- **Giải pháp**: Cần đảm bảo tất cả I2S interfaces đồng bộ với nhau

#### ❌ Buffer Management
- **Vấn đề**: Có thể xảy ra buffer overflow/underflow
- **Giải pháp**: Cải thiện circular buffer handling

### 2. Vấn đề cần cải tiến

#### 🔄 Audio Quality
```c
// Cần cải thiện noise filtering
static void MIC_ARRAY_ApplyFilter(uint16_t *buffer, uint16_t size)
{
    // TODO: Implement digital filtering
    // - High-pass filter để loại bỏ DC offset
    // - Low-pass filter để loại bỏ high-frequency noise
    // - Notch filter cho 50/60Hz noise
}
```

#### 🔄 Real-time Performance
```c
// Cần optimize processing time
HAL_StatusTypeDef MIC_ARRAY_ProcessRealtime(void)
{
    // TODO: Implement real-time constraints
    // - Reduce processing latency
    // - Optimize memory usage
    // - Implement priority-based processing
}
```

#### 🔄 Beamforming Implementation
```c
// Cần triển khai beamforming
typedef struct {
    float direction_angle;    // Detected direction in degrees
    float confidence;        // Detection confidence (0-1)
    uint32_t timestamp;      // Detection timestamp
} MIC_ARRAY_Direction_t;

HAL_StatusTypeDef MIC_ARRAY_DetectDirection(MIC_ARRAY_HandleTypeDef *hmic, 
                                          uint16_t *multichannel_data, 
                                          MIC_ARRAY_Direction_t *result)
{
    // TODO: Implement direction detection algorithm
    // 1. Calculate cross-correlation between channels
    // 2. Find time delays between microphones  
    // 3. Calculate direction using geometry
    // 4. Apply confidence estimation
    return HAL_ERROR; // Not implemented yet
}
```

## Microphone đã định hướng được âm thanh chưa?

### ❌ **CHƯA** - Direction Detection chưa được triển khai

#### Tình trạng hiện tại:
- **Multi-channel capture**: ✅ Hoạt động (4 channels)
- **Audio processing**: ✅ Convert to mono bằng averaging
- **Direction detection**: ❌ Chưa triển khai
- **Beamforming**: ❌ Chưa triển khai

#### Code hiện tại chỉ làm:
```c
// Chỉ average tất cả channels thành mono
for (uint16_t i = 0; i < output_size; i++) {
    int32_t sum = 0;
    for (uint8_t ch = 0; ch < channels; ch++) {
        sum += input[i * channels + ch];
    }
    output[i] = sum / channels;  // Simple averaging
}
```

#### Cần triển khai để có direction detection:

```c
// Direction detection algorithm (cần implement)
typedef struct {
    float x, y;              // Microphone position (mm)
} MIC_Position_t;

// Microphone array geometry (4-mic square array)
static const MIC_Position_t mic_positions[4] = {
    {-30.0f, -30.0f},  // MIC_0: Bottom-left
    { 30.0f, -30.0f},  // MIC_1: Bottom-right  
    { 30.0f,  30.0f},  // MIC_2: Top-right
    {-30.0f,  30.0f}   // MIC_3: Top-left
};

// TDOA-based direction detection
float MIC_ARRAY_CalculateDirection(uint16_t *channel_data[4], uint16_t samples)
{
    // 1. Calculate cross-correlation between mic pairs
    float tdoa_01 = calculate_tdoa(channel_data[0], channel_data[1], samples);
    float tdoa_02 = calculate_tdoa(channel_data[0], channel_data[2], samples);
    float tdoa_03 = calculate_tdoa(channel_data[0], channel_data[3], samples);
    
    // 2. Convert TDOA to direction angle
    float direction = atan2(tdoa_02, tdoa_01) * 180.0f / M_PI;
    
    return direction;
}
```

### Để triển khai direction detection cần:

1. **Implement TDOA calculation**
   - Cross-correlation giữa các cặp microphone
   - Peak detection để tìm time delay
   - Compensation cho sample rate và distance

2. **Geometry calculation**
   - Định nghĩa vị trí chính xác của từng microphone
   - Convert time delays thành góc direction
   - Calibration cho accuracy

3. **Real-time processing**
   - Sliding window analysis
   - Confidence estimation
   - Noise rejection

4. **Testing và calibration**
   - Test với known sound sources
   - Calibrate cho accuracy
   - Validate trong different environments

## Hướng dẫn sử dụng

### 1. Build và Flash

```bash
# Build project
make clean
make -j4

# Flash to STM32
make flash
# hoặc sử dụng STM32CubeIDE
```

### 2. Test Audio

#### UART Debug (115200 baud)
```
[MIC_ARRAY] MIC_ARRAY_Init called
[MIC_ARRAY] Initialization completed successfully!
[MIC_ARRAY] Starting I2S DMA reception...
[MIC_ARRAY] I2S DMA reception started successfully!
UAC Microphone initialized successfully!
```

#### USB Audio Test
1. Kết nối STM32 với PC qua USB
2. PC sẽ nhận diện "STM32 Microphone"
3. Test recording với audio software (Audacity, etc.)

### 3. Audio Test Modes

Project có sẵn các test modes (tự động chuyển mỗi 5s):
- **Mode 0**: Silence
- **Mode 1**: Sine wave 1kHz  
- **Mode 2**: Square wave 2kHz
- **Mode 3**: White noise

### 4. Configuration Parameters

```c
// Trong main.c - có thể điều chỉnh
mic_array_config.sample_rate = 48000;    // Sample rate
mic_array_config.channels = 4;           // Number of mics
mic_array_config.bits_per_sample = 16;   // Bit depth
mic_array_config.buffer_size = 512;      // Buffer size

uint8_t use_mic_array = 1;  // 1=use mics, 0=use test sound
uint8_t test_mode = 1;      // Test sound mode
```

## API Reference

### Microphone Array Functions

```c
// Initialization
HAL_StatusTypeDef MIC_ARRAY_Init(MIC_ARRAY_HandleTypeDef *hmic, 
                                I2S_HandleTypeDef *hi2s, 
                                MIC_ARRAY_Config_t *config);

// Streaming control
HAL_StatusTypeDef MIC_ARRAY_StartStreaming(MIC_ARRAY_HandleTypeDef *hmic);
HAL_StatusTypeDef MIC_ARRAY_StopStreaming(MIC_ARRAY_HandleTypeDef *hmic);

// Data processing
HAL_StatusTypeDef MIC_ARRAY_ReadData(MIC_ARRAY_HandleTypeDef *hmic, 
                                   uint16_t *buffer, uint16_t size);
HAL_StatusTypeDef MIC_ARRAY_ProcessData(MIC_ARRAY_HandleTypeDef *hmic,
                                      uint16_t *input_buffer,
                                      uint16_t *output_buffer, 
                                      uint16_t input_size);

// Configuration
HAL_StatusTypeDef MIC_ARRAY_SetGain(MIC_ARRAY_HandleTypeDef *hmic, uint8_t gain);
uint8_t MIC_ARRAY_GetStatus(MIC_ARRAY_HandleTypeDef *hmic);
```

### USB Audio Functions

```c
// USB Audio initialization
HAL_StatusTypeDef UAC_Init(UAC_HandleTypeDef *huac, 
                          USBD_HandleTypeDef *pdev, 
                          UAC_AudioConfig_t *config);

// Audio test functions
HAL_StatusTypeDef UAC_StartAudioTest(UAC_HandleTypeDef *huac, uint8_t test_mode);
HAL_StatusTypeDef UAC_GenerateTestSignal(UAC_HandleTypeDef *huac, 
                                       uint32_t frequency, 
                                       uint16_t amplitude, 
                                       uint32_t duration_ms);
HAL_StatusTypeDef UAC_ProcessAudioData(UAC_HandleTypeDef *huac, 
                                     uint16_t *data, uint16_t length);
```

## Troubleshooting

### 1. Compilation Issues
```bash
# Math library linking error
arm-none-eabi-gcc: error: -lm: No such file or directory
# Solution: Add -lm to linker flags in Makefile
```

### 2. USB Issues
```
# Device not recognized
# Check: USB descriptor configuration
# Check: Cable connection
# Check: USB power supply
```

### 3. Audio Issues  
```
# No audio data
# Check: I2S configuration
# Check: DMA setup
# Check: Microphone connections
# Check: Clock synchronization
```

### 4. Debug Commands
```c
// Enable debug output
printf("[DEBUG] Buffer: %d samples, Status: %d\r\n", size, status);

// Check DMA status
printf("[DEBUG] DMA Half: %d, Full: %d\r\n", dma_half_complete, dma_full_complete);

// Check audio levels
printf("[DEBUG] Audio levels: %d, %d, %d, %d\r\n", 
       buffer[0], buffer[1], buffer[2], buffer[3]);
```

## Roadmap

### Phase 1: ✅ Hoàn thành
- [x] Basic microphone array capture
- [x] USB Audio Class integration  
- [x] Multi-channel to mono conversion
- [x] Audio test functions
- [x] Debug infrastructure

### Phase 2: 🔄 Đang triển khai
- [ ] Direction detection algorithms
- [ ] Beamforming implementation
- [ ] Audio quality improvements
- [ ] Real-time performance optimization

### Phase 3: 📋 Kế hoạch
- [ ] Advanced noise cancellation
- [ ] Adaptive beamforming
- [ ] Multiple direction tracking
- [ ] Machine learning integration

## Tài liệu tham khảo

- [STM32F411CEUx Datasheet](https://www.st.com/resource/en/datasheet/stm32f411ce.pdf)
- [STM32F4 HAL Reference Manual](https://www.st.com/resource/en/reference_manual/rm0383-stm32f411xce-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)  
- [USB Audio Class Specification](https://www.usb.org/document-library/audio-devices-rev-30-and-adopters-agreement)
- [I2S Protocol Specification](https://www.nxp.com/docs/en/user-guide/UM10732.pdf)
- [Microphone Array Signal Processing](https://link.springer.com/book/10.1007/978-3-662-04619-7)

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contributing

1. Fork the repository
2. Create feature branch (`git checkout -b feature/direction-detection`)
3. Commit changes (`git commit -am 'Add direction detection'`)
4. Push to branch (`git push origin feature/direction-detection`)
5. Create Pull Request

## Contact

**Author:** Nhan Vo  
**Email:** [your-email@example.com]  
**Project:** STM32F4 Microphone Array  
**Date:** September 17, 2025
