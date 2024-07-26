/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <freertos/ringbuf.h>

#include <driver/i2s.h>
#include <driver/gpio.h>
#include "esp_wn_iface.h"
#include "esp_wn_models.h"
#include "dl_lib_coefgetter_if.h"
#include "esp_afe_sr_models.h"
#include "esp_mn_iface.h"
#include "esp_mn_models.h"
#include "model_path.h"
#include "esp_nsn_models.h"
#include "model_path.h"
#include "main.h"

int detect_flag = 0;
static esp_afe_sr_iface_t *afe_handle = NULL;
static esp_afe_sr_data_t *afe_data = NULL;
static RingbufHandle_t ring_buffer;

void feed_Task(void *arg)
{   
    size_t offSet=0;

    esp_afe_sr_data_t *afe_data = arg;
    int audio_chunksize = afe_handle->get_feed_chunksize(afe_data);
    int nch = afe_handle->get_total_channel_num(afe_data);
    
    int feed_channel=1;
    assert(nch <= feed_channel);

    printf("Audio Chunk Size feed: %d\n", audio_chunksize);
    int16_t *i2s_buff = malloc(audio_chunksize * sizeof(int16_t)*feed_channel );
    if (i2s_buff == NULL) {
        printf("Failed to allocate memory for i2s_buff\n");
        return;
    }

    while (true) {
      
        if(PLAY_RM)
        {
        memcpy(i2s_buff, convert_audio_data(&audio_data[offSet],audio_chunksize*2*sizeof(int8_t)), audio_chunksize  * sizeof(int16_t));
        if (offSet >= audio_data_len* sizeof(int8_t)-audio_chunksize) 
        {
            offSet = 0;
        }
        offSet+=audio_chunksize;
        }

        else{
        size_t bytes_read;
        i2s_read(I2S_MIC_NUM, (void *)i2s_buff, audio_chunksize * sizeof(int16_t)*feed_channel, &bytes_read, portMAX_DELAY);
        // printf("Bytes read from I2S: %d\n", bytes_read);  
        }
        
        afe_handle->feed(afe_data, i2s_buff);
    }
    if (i2s_buff) {
        free(i2s_buff);
        i2s_buff = NULL;
    }
}

void play_Task(void *arg)
{  
  size_t offset=0;
  size_t bytes_written;
  size_t count=0;
  esp_afe_sr_data_t *afe_data = arg;
  int afe_chunksize = afe_handle->get_fetch_chunksize(afe_data);
  printf("Audio Chunk Size: %d\n", afe_chunksize);
    
  int16_t *buff = malloc(afe_chunksize * sizeof(int16_t));
  assert(buff);

    while (true) 
    {
      afe_fetch_result_t* res = afe_handle->fetch(afe_data); 

        if (res && res->ret_value != ESP_FAIL) {
            memcpy(&buff[offset], res->data, afe_chunksize * sizeof(int16_t));
            // Adjust volume
            // adjust_volume(buff, afe_chunksize, 0.25);
            
            i2s_write(I2S_SPK_NUM, buff, afe_chunksize* sizeof(int16_t), &bytes_written, portMAX_DELAY);
             
            // printf("Bytes written to I2S: %d\n", bytes_written);
            if(count==100)
            { afe_handle->reset_buffer(afe_data);
            count=0;
            }
            count+=1;

        }
    }
   
    if (buff) {
        free(buff);
        buff = NULL;
    }
    vTaskDelete(NULL);
}

void audio_loop_play_back_Task(void *arg)
{  
   
   size_t bytes_read = 0;
   size_t bytes_written = 0;
   size_t offset=0;
   
  int16_t *i2s_buff = malloc(BUFFER_LENGTH * sizeof(int16_t) );
  if (i2s_buff == NULL) {
        printf("Failed to allocate memory for i2s_buff\n");
        return;
    }

    while (true) 
    {

    if( PLAY_RM)
    {
      if(offset>=audio_data_len-BUFFER_LENGTH* sizeof(int16_t))
      {  
        offset=0;
        printf("full");
      }
      memcpy(i2s_buff,convert_audio_data(&audio_data[offset],BUFFER_LENGTH*2*sizeof(int8_t)), BUFFER_LENGTH * sizeof(int16_t));
      offset+=BUFFER_LENGTH;
      bytes_read=BUFFER_LENGTH;
      // Write to ring buffer
      xRingbufferSend(ring_buffer, (void *)i2s_buff, bytes_read, portMAX_DELAY);
    }
    else
    {
      i2s_read(I2S_MIC_NUM, i2s_buff, BUFFER_LENGTH * sizeof(int16_t), &bytes_read, portMAX_DELAY);
      // printf("Bytes read from I2S: %d\n", bytes_read);
      // Write to ring buffer
      xRingbufferSend(ring_buffer, (void *)i2s_buff, bytes_read, portMAX_DELAY);
    }
    
      // Check if ring buffer is filled with at least twice the buffer length
        if (xRingbufferGetCurFreeSize(ring_buffer) <= RING_BUFFER_SIZE / 2) {
            size_t item_size;
            int16_t *data = (int16_t *)xRingbufferReceiveUpTo(ring_buffer, &item_size, portMAX_DELAY, BUFFER_LENGTH * sizeof(int16_t) * 2);
            if (data != NULL) {
                // Write to I2S
                i2s_write(I2S_SPK_NUM,(void *)data, item_size, &bytes_written, portMAX_DELAY);
                vRingbufferReturnItem(ring_buffer, (void *)data);
            }
        }
}


if (i2s_buff) {
        free(i2s_buff);
        i2s_buff = NULL;
    }
    vTaskDelete(NULL);
}

void app_main()
{
    init_gpio();
    gpio_set_level(GPIO_NUM_9, 1);
    gpio_set_level(GPIO_NUM_8, 0);

    printf("Multiplayer Started!\n");
    i2s_init();
    // Initialize AFE Handler
    afe_handle = (esp_afe_sr_iface_t *)&ESP_AFE_VC_HANDLE;
    
    ring_buffer = xRingbufferCreate(RING_BUFFER_SIZE, RINGBUF_TYPE_BYTEBUF);
    if (ring_buffer == NULL) {
        printf("Failed to create ring buffer\n");
        return;
    }

    afe_config_t afe_config = AFE_CONFIG_DEFAULT();
    afe_config.aec_init = false;
    afe_config.se_init = false;  // Speech enhancement
    afe_config.vad_init = true;
    afe_config.wakenet_init = false;
    afe_config.voice_communication_init = true;
    afe_config.voice_communication_agc_init=true;
    afe_config.voice_communication_agc_gain=50;
    afe_config.agc_mode=AFE_MN_PEAK_AGC_MODE_3;
    afe_config.pcm_config.total_ch_num = 1;
    afe_config.pcm_config.mic_num = 0;
    afe_config.pcm_config.ref_num = 0;
    // // config for nsnet
    afe_config.afe_ns_mode = NS_MODE_NET;
    afe_config.afe_ringbuf_size=15;
    afe_config.memory_alloc_mode=AFE_MEMORY_ALLOC_INTERNAL_PSRAM_BALANCE;
    // char *nsnet_name = esp_srmodel_filter(models, ESP_NSNET_PREFIX, NULL);
    // afe_config.afe_ns_model_name = nsnet_name;

    afe_data = afe_handle->create_from_config(&afe_config);
    if (afe_data == NULL) {
        printf("create_from_config fail!\n");
        return;
    }
    
    xTaskCreatePinnedToCore(&play_Task, "Play Audio", 8 * 1024, (void*)afe_data, 5, NULL, 1);
    xTaskCreatePinnedToCore(&feed_Task, "feed Audio", 8 * 1024, (void*)afe_data, 5, NULL, 0);
    // xTaskCreatePinnedToCore(&audio_loop_play_back_Task, "Loop_Back_Audio_for_Test", 8 * 1024, NULL, 5, NULL, 1);
    
    // printf("destroy\n");
    // afe_handle->destroy(afe_data);
    // afe_data = NULL;
    // printf("successful\n");
}


 void i2s_init()
{
  i2s_config_t i2s_config ={
    .mode= (I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate=SAMPLE_RATE,
    .bits_per_sample= 16,
    .channel_format=I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format=( I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags=ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count=8,
    .dma_buf_len= BUFFER_LENGTH,
    .use_apll=false
    // .tx_desc_auto_clear=true,
    // .fixed_mclk=0
  };
  i2s_pin_config_t pin_config={
    .bck_io_num=I2S_SCK,
    .ws_io_num=I2S_WS,
    .data_out_num=I2S_SD,
    .data_in_num=I2S_PIN_NO_CHANGE
  };
  i2s_driver_install(I2S_SPK_NUM,&i2s_config,0,NULL);
  i2s_set_pin(I2S_SPK_NUM,&pin_config);
  i2s_set_sample_rates(I2S_SPK_NUM,SAMPLE_RATE );


  /*I2s MIC with PCM Pin COnfiguration and driver install*/
  const i2s_config_t i2s_config_mic={
    .mode= (I2S_MODE_MASTER | I2S_MODE_RX ),
    .sample_rate=SAMPLE_RATE,
    .bits_per_sample= 16,
    .channel_format=I2S_CHANNEL_FMT_RIGHT_LEFT,
    // .channel_format=I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format=(i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    // .communication_format=(i2s_comm_format_t)(I2S_COMM_FORMAT_PCM_SHORT),
    .intr_alloc_flags=ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count=8,
    .dma_buf_len= BUFFER_LENGTH,
    .use_apll=false
    // .tx_desc_auto_clear=true,
    // .fixed_mclk=-1
  };
   i2s_pin_config_t pin_config_mic={
    .bck_io_num=I2S_SCK_MIC,
    .ws_io_num=I2S_WS_MIC,
    .data_out_num=I2S_PIN_NO_CHANGE,
    .data_in_num=I2S_SD_MIC
  };
  i2s_driver_install(I2S_MIC_NUM,&i2s_config_mic,0,NULL);
  i2s_set_pin(I2S_MIC_NUM,&pin_config_mic);
}

int16_t* convert_audio_data(const uint8_t *audio_data, size_t audio_data_len) {

  // Step 1: Convert from uint8_t to uint16_t
  int16_t *audio_data_uint16 = (int16_t *)malloc(audio_data_len / 2 * sizeof(int16_t));
  for (size_t i = 0; i < audio_data_len / 2; i++) 
  {
    audio_data_uint16[i] = (audio_data[2 * i + 1] << 8) | audio_data[2 * i];
  }

  return audio_data_uint16;
}

void init_gpio() {
    gpio_config_t io_conf;

    // Configure GPIO 9 and 8 as output
    io_conf.intr_type = GPIO_INTR_DISABLE;   // Disable interrupt
    io_conf.mode = GPIO_MODE_OUTPUT;         // Set as output mode
    io_conf.pin_bit_mask = (1ULL<<9); // Bit mask of the pins to set
    io_conf.pull_down_en = 0;                // Disable pull-down
    io_conf.pull_up_en = 0;                  // Disable pull-up

    // Configure the GPIO with the given settings
    gpio_config(&io_conf);
}
// Function to adjust volume
void adjust_volume(int16_t *buffer, size_t length, float volume_factor) {
    for (size_t i = 0; i < length; i++) {
        buffer[i] = buffer[i] * volume_factor;
    }
}
// Function to extract left channel

