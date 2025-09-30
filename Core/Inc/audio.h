#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "main.h"
#include "stm32f4xx_hal.h"

// Audio buffer size
#define AUDIO_BUFFER_SIZE 512

// USB Audio configuration
#define USB_AUDIO_SAMPLE_RATE 48000
#define USB_AUDIO_PACKET_SIZE 96  // 48 samples * 2 bytes per sample

// Audio modes
typedef enum {
    AUDIO_MODE_IDLE = 0,
    AUDIO_MODE_LIVE = 1
} AudioMode_t;

// ================= Public API =============================
AudioMode_t Audio_GetMode(void);
uint16_t Audio_GetVolume(void);
uint8_t Audio_GetMute(void);

// Sample Rate Control
uint32_t Audio_Get_Sample_Rate(void);
HAL_StatusTypeDef Audio_Set_Sample_Rate(uint32_t sample_rate);

// ================= Audio Control ==========================
HAL_StatusTypeDef Audio_Init_Live_Mode(void);
HAL_StatusTypeDef Audio_Start_Live_Mode(void);
HAL_StatusTypeDef Audio_Stop_Live_Mode(void);
void Audio_Mute(void);
void Audio_UnMute(void);
void Audio_Set_Volume(uint16_t volume);

// ================= USB Audio Functions ====================
void Audio_USB_Start_Streaming(void);
void Audio_USB_Stop_Streaming(void);
void Audio_USB_Process_I2S_Data(uint32_t* i2s_data, uint32_t length);
uint16_t Audio_USB_Get_Next_Packet(uint8_t* buffer, uint16_t max_size);
void Audio_USB_Print_Status(void);
void Audio_Generate_Test_Tone(void);
void Audio_Print_Data_Flow_Status(void);

// ================= I2S Callbacks =======================
// Note: I2S callbacks are implemented in MIC_ARRAY.c to avoid multiple definition

#endif /* __AUDIO_H__ */
