#include "audio_buffer.h"
#include <string.h>

/* Global Audio Buffer */
static AudioBuffer_t audio_buffer;

/**
 * @brief  Initialize Audio Buffer
 * @retval None
 */
void AudioBuffer_Init(void)
{
    memset(&audio_buffer, 0, sizeof(AudioBuffer_t));
    audio_buffer.write_ptr = 0;
    audio_buffer.read_ptr = 0;
    audio_buffer.level = 0;
    audio_buffer.ready = 0;
    audio_buffer.overruns = 0;
    audio_buffer.underruns = 0;
    
    printf("[AUDIO_BUFFER] Initialized - Size: %d, Packets: %d\r\n", 
           AUDIO_BUFFER_SIZE, AUDIO_BUFFER_PACKETS);
}

/**
 * @brief  Write audio data to buffer
 * @param  data: Pointer to audio data
 * @param  length: Number of samples to write
 * @retval None
 */
void AudioBuffer_Write(int16_t* data, uint16_t length)
{
    if (data == NULL || length == 0) return;
    
    for (uint16_t i = 0; i < length; i++) {
        // Check for buffer overflow
        if (audio_buffer.level >= AUDIO_BUFFER_SIZE) {
            audio_buffer.overruns++;
            // Advance read pointer to make space
            audio_buffer.read_ptr = (audio_buffer.read_ptr + 1) % AUDIO_BUFFER_SIZE;
            audio_buffer.level--;
        }
        
        // Write sample to buffer
        audio_buffer.buffer[audio_buffer.write_ptr] = data[i];
        audio_buffer.write_ptr = (audio_buffer.write_ptr + 1) % AUDIO_BUFFER_SIZE;
        audio_buffer.level++;
        
        // Set ready flag when buffer has enough data for one packet
        if (audio_buffer.level >= AUDIO_PACKET_SIZE) {
            audio_buffer.ready = 1;
        }
    }
}

/**
 * @brief  Read audio data from buffer
 * @param  data: Pointer to output buffer
 * @param  max_length: Maximum number of samples to read
 * @retval Number of samples actually read
 */
uint16_t AudioBuffer_Read(int16_t* data, uint16_t max_length)
{
    if (data == NULL || max_length == 0) return 0;
    
    uint16_t samples_to_read = (audio_buffer.level < max_length) ? 
                               audio_buffer.level : max_length;
    
    if (samples_to_read == 0) {
        audio_buffer.underruns++;
        return 0;
    }
    
    for (uint16_t i = 0; i < samples_to_read; i++) {
        data[i] = audio_buffer.buffer[audio_buffer.read_ptr];
        audio_buffer.read_ptr = (audio_buffer.read_ptr + 1) % AUDIO_BUFFER_SIZE;
        audio_buffer.level--;
    }
    
    // Clear ready flag if buffer level is too low
    if (audio_buffer.level < AUDIO_PACKET_SIZE) {
        audio_buffer.ready = 0;
    }
    
    return samples_to_read;
}

/**
 * @brief  Get current buffer level
 * @retval Current buffer level in samples
 */
uint16_t AudioBuffer_GetLevel(void)
{
    return audio_buffer.level;
}

/**
 * @brief  Check if buffer is ready for reading
 * @retval 1 if ready, 0 if not ready
 */
uint8_t AudioBuffer_IsReady(void)
{
    return audio_buffer.ready;
}

/**
 * @brief  Reset audio buffer
 * @retval None
 */
void AudioBuffer_Reset(void)
{
    audio_buffer.write_ptr = 0;
    audio_buffer.read_ptr = 0;
    audio_buffer.level = 0;
    audio_buffer.ready = 0;
    audio_buffer.overruns = 0;
    audio_buffer.underruns = 0;
    
    printf("[AUDIO_BUFFER] Reset completed\r\n");
}
