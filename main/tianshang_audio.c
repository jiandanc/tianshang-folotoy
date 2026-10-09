#include "tianshang_audio.h"
#include <string.h>

#ifdef ESP_PLATFORM
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "bsp_audio.h"
#include "tianshang_sfx_data.h"

static const char *TAG = "tianshang_audio";

#define AUDIO_SAMPLE_RATE    16000

static QueueHandle_t s_audio_queue = NULL;
static TaskHandle_t s_audio_task = NULL;
static bool s_audio_ready = false;

static void audio_worker_task(void *arg)
{
    (void)arg;
    tianshang_sfx_t sfx;

    while (1) {
        if (xQueueReceive(s_audio_queue, &sfx, portMAX_DELAY) == pdTRUE) {
            const int16_t *buf = NULL;
            size_t bytes = 0;

            switch (sfx) {
                case TIANSHANG_SFX_CLICK:
                    buf = s_pcm_click;
                    bytes = sizeof(s_pcm_click);
                    break;
                case TIANSHANG_SFX_FAN:
                    buf = s_pcm_fan;
                    bytes = sizeof(s_pcm_fan);
                    break;
                case TIANSHANG_SFX_QI:
                    buf = s_pcm_qi;
                    bytes = sizeof(s_pcm_qi);
                    break;
                case TIANSHANG_SFX_TIAN:
                    buf = s_pcm_tian;
                    bytes = sizeof(s_pcm_tian);
                    break;
                default:
                    break;
            }

            if (buf && bytes > 0) {
                const uint8_t *p = (const uint8_t *)buf;
                size_t left = bytes;
                while (left > 0) {
                    size_t chunk = (left > 1024) ? 1024 : left;
                    bsp_audio_write(p, chunk);
                    p += chunk;
                    left -= chunk;
                }
            }
        }
    }
}

bool tianshang_audio_init(void)
{
    if (bsp_audio_init() != ESP_OK ||
        bsp_audio_set_format(AUDIO_SAMPLE_RATE, 16, 1) != ESP_OK) {
        ESP_LOGW(TAG, "Audio codec unavailable, continuing muted");
        s_audio_ready = false;
        return false;
    }

    bsp_audio_set_volume(75);

    s_audio_queue = xQueueCreate(8, sizeof(tianshang_sfx_t));
    if (!s_audio_queue) {
        ESP_LOGW(TAG, "Failed to create audio queue");
        return false;
    }

    if (xTaskCreate(audio_worker_task, "ts_audio", 3072, NULL, 5, &s_audio_task) != pdPASS) {
        ESP_LOGW(TAG, "Failed to create audio task");
        vQueueDelete(s_audio_queue);
        s_audio_queue = NULL;
        return false;
    }

    s_audio_ready = true;
    ESP_LOGI(TAG, "Audio initialized successfully");
    return true;
}

void tianshang_audio_play(tianshang_sfx_t sfx)
{
    if (s_audio_ready && s_audio_queue) {
        xQueueSend(s_audio_queue, &sfx, 0);
    }
}

#else

// Host mockup stubs
bool tianshang_audio_init(void) { return true; }
void tianshang_audio_play(tianshang_sfx_t sfx) { (void)sfx; }

#endif
