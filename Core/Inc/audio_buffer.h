#ifndef __AUDIO_BUFFER_H__
#define __AUDIO_BUFFER_H__

#include "main.h"

/* Audio Buffer Configuration */
#define AUDIO_BUFFER_SIZE         1024    /* Total buffer size in samples */
#define AUDIO_PACKET_SIZE         48      /* USB Audio packet size in samples */
#define AUDIO_BUFFER_PACKETS      (AUDIO_BUFFER_SIZE / AUDIO_PACKET_SIZE)

/* Audio Buffer Structure */
typedef struct {
    int16_t buffer[AUDIO_BUFFER_SIZE];     /* Audio data buffer */
    volatile uint16_t write_ptr;           /* Write pointer */
    volatile uint16_t read_ptr;            /* Read pointer */
    volatile uint16_t level;               /* Current buffer level */
    volatile uint8_t ready;                /* Buffer ready flag */
    volatile uint32_t overruns;            /* Buffer overrun counter */
    volatile uint32_t underruns;           /* Buffer underrun counter */
} AudioBuffer_t;

/* Function Prototypes */
void AudioBuffer_Init(void);
void AudioBuffer_Write(int16_t* data, uint16_t length);
uint16_t AudioBuffer_Read(int16_t* data, uint16_t max_length);
uint16_t AudioBuffer_GetLevel(void);
uint8_t AudioBuffer_IsReady(void);
void AudioBuffer_Reset(void);

#endif /* __AUDIO_BUFFER_H__ */
