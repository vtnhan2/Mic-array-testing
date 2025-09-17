# STM32F4 Microphone Array Project

**Author:** Nhan Vo  
**Date:** September 17, 2025  
**Version:** 1.0

## Overview

This project implements a microphone array system using STM32F411CEUx with multi-channel audio capture capability and USB Audio Class data transmission. The system supports 4 microphones connected via I2S and can operate as a USB microphone.

## Key Features

- ✅ **4-channel microphone array** with I2S interface
- ✅ **USB Audio Class** - operates as USB microphone  
- ✅ **Real-time audio processing** with DMA
- ✅ **Multi-channel to mono conversion**
- ✅ **Gain control** (0-100%)
- ✅ **Audio test functions** with sine wave, square wave, noise
- ✅ **Debug output** via UART2
- ❌ **Audio direction detection** (not implemented yet)

## Project Structure

```
mic_array_stm32/
├── Core/
│   ├── Inc/
│   │   ├── main.h              # Main header with GPIO definitions
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

## Hardware Configuration

### STM32F411CEUx Pin Configuration

#### I2S Interfaces (4 microphones)

| Microphone | I2S | Clock Pin | WS Pin | Data Pin | Mode |
|------------|-----|-----------|--------|----------|------|
| MIC_0 | I2S1 | PA5 (mic_clk_0) | PA4 (mic_ws_0) | PA7 (mic_dout_0) | Slave RX |
| MIC_1 | I2S2 | PB10 (mic_clk_1) | PB9 (mic_ws_1) | PB15 (mic_dout_1) | Master RX |
| MIC_2 | I2S4 | PB13 (mic_clk_2) | PB12 (mic_ws_2) | PA1 (mic_dout_2) | Slave RX |
| MIC_3 | I2S5 | PB0 (mic_clk_3) | PB1 (mic_ws_3) | PA10 (mic_dout_3) | Slave RX |

#### Other Pins

| Function | Pin | Description |
|----------|-----|-------------|
| USB D- | PA11 | USB OTG FS Data - |
| USB D+ | PA12 | USB OTG FS Data + |
| UART TX | PA2 | Debug output |
| UART RX | PA3 | Debug input |
| LED Clock | PB3 | SPI3 SCK for LED control |
| LED Data | PB5 | SPI3 MOSI for LED control |
| SWDIO | PA13 | Debug interface |
| SWCLK | PA14 | Debug interface |

### Microphone Array to STM32F4 Connection

#### Basic Connection Diagram

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

#### Connection Notes

1. **I2S2 is Master**: Provides clock for all microphones
2. **I2S1, I2S4, I2S5 are Slaves**: Receive clock from I2S2 or external source
3. **Power Supply**: 3.3V from STM32
4. **Ground**: Common GND connection

## Configuring STM32 as Microphone

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

## Issues to Address

### 1. Current Issues

#### ❌ Audio Direction Detection Not Implemented
- **Problem**: Current code only converts multi-channel to mono by averaging
- **Solution Needed**: 
  - Beamforming algorithms
  - Time Difference of Arrival (TDOA)
  - Cross-correlation analysis
  - Direction of Arrival (DOA) estimation

#### ❌ Incomplete GPIO Configuration
- **Problem**: `MIC_ARRAY_GPIO_Init()` function is empty
- **Solution**: GPIO already configured in STM32CubeMX, function kept for compatibility

#### ❌ I2S Synchronization
- **Problem**: Only I2S2 is master, other I2S interfaces need sync
- **Solution**: Ensure all I2S interfaces are synchronized

#### ❌ Buffer Management
- **Problem**: Potential buffer overflow/underflow
- **Solution**: Improve circular buffer handling

### 2. Areas for Improvement

#### 🔄 Audio Quality
```c
// Need to improve noise filtering
static void MIC_ARRAY_ApplyFilter(uint16_t *buffer, uint16_t size)
{
    // TODO: Implement digital filtering
    // - High-pass filter to remove DC offset
    // - Low-pass filter to remove high-frequency noise
    // - Notch filter for 50/60Hz noise
}
```

#### 🔄 Real-time Performance
```c
// Need to optimize processing time
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
// Need to implement beamforming
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

## Has the Microphone Array Achieved Audio Direction Detection?

### ❌ **NO** - Direction Detection Not Implemented Yet

#### Current Status:
- **Multi-channel capture**: ✅ Working (4 channels)
- **Audio processing**: ✅ Convert to mono by averaging
- **Direction detection**: ❌ Not implemented
- **Beamforming**: ❌ Not implemented

#### Current code only does:
```c
// Only averages all channels to mono
for (uint16_t i = 0; i < output_size; i++) {
    int32_t sum = 0;
    for (uint8_t ch = 0; ch < channels; ch++) {
        sum += input[i * channels + ch];
    }
    output[i] = sum / channels;  // Simple averaging
}
```

#### To achieve direction detection, need to implement:

```c
// Direction detection algorithm (needs implementation)
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

### To implement direction detection, need:

1. **Implement TDOA calculation**
   - Cross-correlation between microphone pairs
   - Peak detection to find time delay
   - Compensation for sample rate and distance

2. **Geometry calculation**
   - Define precise position of each microphone
   - Convert time delays to direction angle
   - Calibration for accuracy

3. **Real-time processing**
   - Sliding window analysis
   - Confidence estimation
   - Noise rejection

4. **Testing and calibration**
   - Test with known sound sources
   - Calibrate for accuracy
   - Validate in different environments

## Usage Guide

### 1. Build and Flash

```bash
# Build project
make clean
make -j4

# Flash to STM32
make flash
# or use STM32CubeIDE
```

### 2. Audio Testing

#### UART Debug (115200 baud)
```
[MIC_ARRAY] MIC_ARRAY_Init called
[MIC_ARRAY] Initialization completed successfully!
[MIC_ARRAY] Starting I2S DMA reception...
[MIC_ARRAY] I2S DMA reception started successfully!
UAC Microphone initialized successfully!
```

#### USB Audio Test
1. Connect STM32 to PC via USB
2. PC will recognize "STM32 Microphone"
3. Test recording with audio software (Audacity, etc.)

### 3. Audio Test Modes

Project includes test modes (auto-switch every 5s):
- **Mode 0**: Silence
- **Mode 1**: Sine wave 1kHz  
- **Mode 2**: Square wave 2kHz
- **Mode 3**: White noise

### 4. Configuration Parameters

```c
// In main.c - adjustable
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

### Phase 1: ✅ Completed
- [x] Basic microphone array capture
- [x] USB Audio Class integration  
- [x] Multi-channel to mono conversion
- [x] Audio test functions
- [x] Debug infrastructure

### Phase 2: 🔄 In Progress
- [ ] Direction detection algorithms
- [ ] Beamforming implementation
- [ ] Audio quality improvements
- [ ] Real-time performance optimization

### Phase 3: 📋 Planned
- [ ] Advanced noise cancellation
- [ ] Adaptive beamforming
- [ ] Multiple direction tracking
- [ ] Machine learning integration

## References

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
