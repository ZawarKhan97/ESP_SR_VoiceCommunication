#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <stddef.h> // Include for NULL definition
#include <audio_data.h>
#include <esp_afe_sr_iface.h>
#include <esp_afe_sr_models.h>
#include <esp_agc.h>

/* for my dev Kit */
#define I2S_WS  41 
#define I2S_SD  39
#define I2S_SCK 40
/*Pins for MIC PCM*/
#define I2S_WS_MIC  11
#define I2S_SD_MIC  10
#define I2S_SCK_MIC 12

// /*Pins for MIC PCM Connecto*/
// #define I2S_WS_MIC  10
// #define I2S_SD_MIC  11
// #define I2S_SCK_MIC 10
// I2S port
#define I2S_MIC_NUM       I2S_NUM_0  // I2S port number for the microphone
#define I2S_SPK_NUM       I2S_NUM_1  // I2S port number for the speaker

// Buffer length
#define BUFFER_LENGTH 160
#define RING_BUFFER_SIZE (BUFFER_LENGTH * 2 * sizeof(int16_t) * 2) 

// AGC enable flag
#define AGC_ENABLE 0
#define PLAY_RM 0
/* COnfiguration Parameters*/
#define SAMPLE_RATE 16000
#define AGC_MODE 3
#define Gain_dB 100
#define LIMITER_ENABLE 0
#define PRINT 0

const int TARGET_LEVEL=5;
/* AFE Handle Configuration*/
// static esp_afe_sr_iface_t *afe_handle=NULL;
// // static esp_afe_sr_data_t *afe_data=NULL;
// void* agc_handle;
/* AGC attributes config*/

/*Function Prototypes*/
void i2s_init();
void init_gpio();
void play_audio(void *arg);
int16_t* convert_audio_data(const uint8_t *audio_data, size_t audio_data_len);
void adjust_volume(int16_t *buffer, size_t length, float volume_factor);
void extract_left_channel(const int16_t *interleaved_buffer, int16_t *left_channel, size_t length);

#endif // MAIN_H