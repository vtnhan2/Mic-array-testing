#include "audio.h"
#include "UAC.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

// These handles are created in main.c by CubeMX
extern I2S_HandleTypeDef hi2s1;
extern I2S_HandleTypeDef hi2s2;
extern UART_HandleTypeDef huart2;

// ================= Module state ===========================
static volatile uint32_t rxBuffer[AUDIO_BUFFER_SIZE];
static volatile uint32_t txBuffer[AUDIO_BUFFER_SIZE];

static AudioMode_t audioMode = AUDIO_MODE_IDLE;
static uint16_t volumeValue = 50;
static uint32_t audioSampleCount = 0;
static uint8_t audioMuteState = 1; // 1 = muted, 0 = unmuted

// Arduino-like real-time helpers kept from original
// static uint8_t  decimationCounter = 0;  // Unused
// static uint16_t txIndex = 0, rxIndex = 0;  // Unused
static uint32_t totalSampleCount = 0;

// Improved audio processing state
static volatile uint32_t processed_samples = 0;
static volatile uint32_t usb_samples_sent = 0;
static volatile uint8_t audio_quality_monitor = 0;

// USB Audio streaming buffers and state - Optimized for smooth playback
#define USB_RING_BUFFER_SIZE (USB_AUDIO_PACKET_SIZE * 16) // Balanced buffer for smooth audio
static int16_t usb_audio_buffer[USB_RING_BUFFER_SIZE];
static volatile uint16_t usb_buffer_write_ptr = 0;
static volatile uint16_t usb_buffer_read_ptr = 0;
static volatile uint16_t usb_buffer_level = 0;
static volatile uint8_t usb_streaming_active = 0;
static volatile uint32_t buffer_overruns = 0;
static volatile uint32_t buffer_underruns = 0;

// ================= Public API =============================
AudioMode_t Audio_GetMode(void)   { return audioMode; }
uint16_t    Audio_GetVolume(void) { return volumeValue; }
uint8_t     Audio_GetMute(void)   { return audioMuteState; }

// Sample Rate Control
static uint32_t current_sample_rate = USB_AUDIO_SAMPLE_RATE;

uint32_t Audio_Get_Sample_Rate(void) 
{ 
    return current_sample_rate; 
}

HAL_StatusTypeDef Audio_Set_Sample_Rate(uint32_t sample_rate)
{
    // Validate sample rate
    switch(sample_rate) {
        case 8000:
        case 16000:
        case 22050:
        case 44100:
        case 48000:
        case 96000:
            break;
        default:
            return HAL_ERROR; // Invalid sample rate
    }
    
    // Stop audio if running
    uint8_t was_running = (audioMode == AUDIO_MODE_LIVE);
    if (was_running) {
        Audio_Stop_Live_Mode();
    }
    
    // Update sample rate
    current_sample_rate = sample_rate;
    
    // Reconfigure I2S with new sample rate
    uint32_t i2s_freq;
    switch(sample_rate) {
        case 8000:  i2s_freq = I2S_AUDIOFREQ_8K; break;
        case 16000: i2s_freq = I2S_AUDIOFREQ_16K; break;
        case 22050: i2s_freq = I2S_AUDIOFREQ_22K; break;
        case 44100: i2s_freq = I2S_AUDIOFREQ_44K; break;
        case 48000: i2s_freq = I2S_AUDIOFREQ_48K; break;
        case 96000: i2s_freq = I2S_AUDIOFREQ_96K; break;
        default: return HAL_ERROR;
    }
    
    // Update I2S configuration
    hi2s1.Init.AudioFreq = i2s_freq;
    hi2s2.Init.AudioFreq = i2s_freq;
    
    // Restart audio if it was running
    if (was_running) {
        Audio_Start_Live_Mode();
    }
    
    return HAL_OK;
}

// ================= Audio Control ==========================
HAL_StatusTypeDef Audio_Init_Live_Mode(void)
{
    // Initialize audio buffers
    memset((void*)rxBuffer, 0, sizeof(rxBuffer));
    memset((void*)txBuffer, 0, sizeof(txBuffer));
    
    // Reset audio state
    audioMode = AUDIO_MODE_IDLE;
    audioSampleCount = 0;
    totalSampleCount = 0;
    
    // Initialize USB audio buffer
    memset(usb_audio_buffer, 0, sizeof(usb_audio_buffer));
    usb_buffer_write_ptr = 0;
    usb_buffer_read_ptr = 0;
    usb_buffer_level = 0;
    usb_streaming_active = 0;
    buffer_overruns = 0;
    buffer_underruns = 0;
    
    return HAL_OK;
}

HAL_StatusTypeDef Audio_Start_Live_Mode(void)
{
    if (audioMode == AUDIO_MODE_LIVE) {
        return HAL_OK; // Already running
    }
    
    // Start I2S reception for microphone (I2S2 is the microphone)
    HAL_StatusTypeDef status = HAL_I2S_Receive_DMA(&hi2s2, (uint16_t*)rxBuffer, AUDIO_BUFFER_SIZE);
    if (status != HAL_OK) {
        printf("[AUDIO] Failed to start I2S2 DMA: %d\r\n", status);
        return status;
    }
    
    printf("[AUDIO] I2S2 DMA started successfully\r\n");
    audioMode = AUDIO_MODE_LIVE;
    return HAL_OK;
}

HAL_StatusTypeDef Audio_Stop_Live_Mode(void)
{
    if (audioMode != AUDIO_MODE_LIVE) {
        return HAL_OK; // Not running
    }
    
    // Stop I2S reception (I2S2 is the microphone)
    HAL_I2S_DMAStop(&hi2s2);
    
    audioMode = AUDIO_MODE_IDLE;
    return HAL_OK;
}

void Audio_Mute(void)
{
    audioMuteState = 1;
}

void Audio_UnMute(void)
{
    audioMuteState = 0;
}

void Audio_Set_Volume(uint16_t volume)
{
    if (volume > 100) volume = 100;
    volumeValue = volume;
}

// ================= USB Audio Functions ====================
void Audio_USB_Start_Streaming(void)
{
    usb_streaming_active = 1;
    usb_buffer_level = 0;
    usb_buffer_write_ptr = 0;
    usb_buffer_read_ptr = 0;
    printf("[AUDIO] USB Streaming STARTED\r\n");
}

void Audio_USB_Stop_Streaming(void)
{
    usb_streaming_active = 0;
}

void Audio_USB_Process_I2S_Data(uint32_t* i2s_data, uint32_t length)
{
    const uint16_t buffer_size = USB_RING_BUFFER_SIZE;
    static uint32_t debug_count = 0;
    
    // Debug: Print first few samples
    if (debug_count < 10) {
        printf("[AUDIO] Processing I2S data: length=%lu, samples=[%08X, %08X, %08X, %08X]\r\n", 
               length, i2s_data[0], i2s_data[1], i2s_data[2], i2s_data[3]);
        debug_count++;
    }
    
    // Check if I2S data is all zeros (silence)
    uint32_t non_zero_count = 0;
    for (uint32_t i = 0; i < length && i < 10; i++) {
        if (i2s_data[i] != 0 && i2s_data[i] != 0xFFFFFFFF) {
            non_zero_count++;
        }
    }
    if (debug_count == 1) {
        printf("[AUDIO] Non-zero samples in first 10: %lu\r\n", non_zero_count);
    }
    
    // Process I2S data and convert to USB audio format
    for (uint32_t i = 0; i < length && usb_buffer_level < buffer_size; i++) {
        uint32_t raw_sample = i2s_data[i];
        
        // Convert I2S data to 16-bit signed sample
        int16_t sample = 0;
        
        // Extract audio data from I2S format (assuming 16-bit left-justified)
        if (raw_sample != 0 && raw_sample != 0xFFFFFFFF) {
            // Convert from I2S format to 16-bit signed
            sample = (int16_t)(raw_sample >> 16);
            
            // Apply mute if needed
            if (audioMuteState) {
                sample = 0;
            }
            
            // Apply volume control
            sample = (int16_t)((sample * volumeValue) / 100);
            
            // Clamp to prevent overflow
            if (sample > 32767) sample = 32767;
            if (sample < -32768) sample = -32768;
            
            // Additional validation
            if (sample == (int16_t)0xFFFF || sample == (int16_t)0x8000) {
                sample = 0; // These are likely overflow artifacts
            }
        }
        
        // Store in USB buffer (ring buffer)
        usb_audio_buffer[usb_buffer_write_ptr] = sample;
        usb_buffer_write_ptr = (usb_buffer_write_ptr + 1) % buffer_size;
        
        // Track buffer level with overflow protection
        if (usb_buffer_level < buffer_size) {
            usb_buffer_level++;
        } else {
            // Buffer overflow - advance read pointer and count overrun
            usb_buffer_read_ptr = (usb_buffer_read_ptr + 1) % buffer_size;
            buffer_overruns++;
        }
    }
}

uint16_t Audio_USB_Get_Next_Packet(uint8_t* buffer, uint16_t max_size)
{
    const uint16_t buffer_size = USB_RING_BUFFER_SIZE;
    static uint32_t debug_count = 0;
    
    // Debug: Print status every 100 calls
    if (debug_count % 100 == 0) {
        printf("[AUDIO] USB Get Packet: Active=%d, Level=%d/%d, MaxSize=%d\r\n", 
               usb_streaming_active, usb_buffer_level, buffer_size, max_size);
    }
    debug_count++;
    
    // Debug: Print first few calls to see if function is called
    if (debug_count <= 5) {
        printf("[AUDIO] USB Get Packet called: count=%lu, Active=%d, Level=%d\r\n", 
               debug_count, usb_streaming_active, usb_buffer_level);
    }
    
    // Lower threshold to reduce choppy audio - send partial data if needed
    // uint16_t min_samples = USB_AUDIO_PACKET_SIZE / 4; // Accept 1/4 packet minimum - unused
    
    if (!usb_streaming_active || usb_buffer_level < USB_AUDIO_PACKET_SIZE) {
        // Not enough data or not streaming - send silence and count underrun
        if (usb_streaming_active && usb_buffer_level < USB_AUDIO_PACKET_SIZE) {
            buffer_underruns++;
        }
        memset(buffer, 0, max_size);
        return max_size;
    }
    
    uint16_t bytes_to_send = USB_AUDIO_PACKET_SIZE * 2; // 16-bit samples
    if (bytes_to_send > max_size) {
        bytes_to_send = max_size;
    }
    
    // Adaptive packet size based on buffer level
    uint16_t available_samples = usb_buffer_level;
    uint16_t samples_to_send = bytes_to_send / 2;
    
    // If we have less than full packet, send what we have to reduce latency
    if (available_samples < samples_to_send) {
        samples_to_send = available_samples;
        bytes_to_send = samples_to_send * 2;
    }
    
    uint8_t* dest = buffer;
    
    for (uint16_t i = 0; i < samples_to_send; i++) {
        int16_t sample = usb_audio_buffer[usb_buffer_read_ptr];
        usb_buffer_read_ptr = (usb_buffer_read_ptr + 1) % buffer_size;
        usb_buffer_level--;
        
        // Additional validation before sending to USB
        // Reject obviously corrupted or extreme values
        if (sample == (int16_t)0xFFFF || sample == (int16_t)0x8000) {
            sample = 0;
        }
        
        // Convert to little-endian bytes
        *dest++ = (uint8_t)(sample & 0xFF);
        *dest++ = (uint8_t)((sample >> 8) & 0xFF);
    }
    
    // Fill remaining space with silence if partial packet
    while (dest < (buffer + max_size)) {
        *dest++ = 0;
    }
    
    usb_samples_sent += samples_to_send;
    return bytes_to_send;
}

void Audio_USB_Print_Status(void)
{
    char status_msg[200];
    int len = snprintf(status_msg, sizeof(status_msg), 
        "USB Audio: Active=%d, Buffer=%d/%d, Samples=%lu, Over=%lu, Under=%lu, TotalSamples=%lu, Muted=%d\r\n",
        usb_streaming_active, usb_buffer_level, USB_RING_BUFFER_SIZE, 
        usb_samples_sent, buffer_overruns, buffer_underruns, totalSampleCount, audioMuteState);
    HAL_UART_Transmit(&huart2, (uint8_t*)status_msg, len, 100);
}

void Audio_Generate_Test_Tone(void)
{
    // Generate a simple test tone to verify USB audio path
    static uint32_t tone_phase = 0;
    const uint16_t test_samples = 96; // One USB packet worth
    int16_t test_buffer[96];
    
    for (uint16_t i = 0; i < test_samples; i++) {
        // Generate 1kHz sine wave at 48kHz sample rate
        float sample = sinf(2.0f * 3.14159f * 1000.0f * tone_phase / 48000.0f);
        test_buffer[i] = (int16_t)(sample * 8000); // Moderate amplitude
        tone_phase++;
    }
    
    // Add test tone to USB buffer
    for (uint16_t i = 0; i < test_samples && usb_buffer_level < USB_RING_BUFFER_SIZE; i++) {
        usb_audio_buffer[usb_buffer_write_ptr] = test_buffer[i];
        usb_buffer_write_ptr = (usb_buffer_write_ptr + 1) % USB_RING_BUFFER_SIZE;
        usb_buffer_level++;
    }
}

void Audio_Print_Data_Flow_Status(void)
{
    char status_msg[300];
    int len = snprintf(status_msg, sizeof(status_msg), 
        "Audio Status: Mode=%d, Samples=%lu, USB_Buffer=%d/%d, Over=%lu, Under=%lu, Muted=%d, Vol=%d\r\n",
        audioMode, totalSampleCount, usb_buffer_level, USB_RING_BUFFER_SIZE, 
        buffer_overruns, buffer_underruns, audioMuteState, volumeValue);
    HAL_UART_Transmit(&huart2, (uint8_t*)status_msg, len, 100);
}

// ================= I2S Callbacks =======================
// Note: I2S callbacks are implemented in MIC_ARRAY.c to avoid multiple definition
