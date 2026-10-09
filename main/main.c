// FoloToy AI Passport - Tianshang Gacha Application
#include <stdint.h>
#include <stdbool.h>

#include "bsp_audio.h"
#include "bsp_battery.h"
#include "bsp_button.h"
#include "bsp_display.h"
#include "bsp_i2c.h"
#include "bsp_pins.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "lvgl.h"

#include "tianshang_sim.h"
#include "tianshang_store.h"
#include "tianshang_ui.h"
#include "tianshang_audio.h"

static const char *TAG = "tianshang_app";

typedef enum {
    APP_EVENT_DRAW_SINGLE = 0,
    APP_EVENT_DRAW_TEN,
    APP_EVENT_TOGGLE_MODE,
    APP_EVENT_RESET
} app_event_t;

#define IDLE_DIM_TIMEOUT_US   (30ULL * 1000ULL * 1000ULL) // 30 秒无操作降低亮度
#define IDLE_SLEEP_TIMEOUT_US (60ULL * 1000ULL * 1000ULL) // 1 分钟无操作深度休眠

#define BACKLIGHT_NORMAL      100
#define BACKLIGHT_DIM         20

static QueueHandle_t s_event_queue = NULL;
static tianshang_state_t s_state;
static lv_timer_t *s_battery_timer = NULL;
static volatile int64_t s_last_activity_us = 0;
static volatile bool s_is_dimmed = false;

static inline void update_activity(void)
{
    s_last_activity_us = esp_timer_get_time();
    if (s_is_dimmed) {
        s_is_dimmed = false;
        bsp_display_backlight(BACKLIGHT_NORMAL);
    }
}

static void battery_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    int soc = bsp_battery_soc();
    tianshang_ui_update_battery(soc);
}

// 抽奖结果先上屏、后落盘:NVS 写入耗时远高于一次刷新，放在 UI 之后
// 不会让按键到画面的延迟变差。失败只记录日志，不影响本次抽奖结果。
static void persist_state(void)
{
    tianshang_store_result_t res = tianshang_store_save(&s_state);
    if (res != TIANSHANG_STORE_OK) {
        ESP_LOGE(TAG, "Failed to persist draw state (%d); records may be lost on reboot", (int)res);
    }
}

static void enter_deep_sleep(void)
{
    ESP_LOGI(TAG, "Inactivity timeout (1 minute) reached; preparing deep sleep");

    // 1. 抽卡状态落盘，防止数据丢失
    persist_state();

    // 2. 按序安全挂起外设 (CW2017 -> ES8311 -> I2S -> 共享 I2C)
    bsp_battery_sleep();
    bsp_audio_sleep();
    bsp_audio_prepare_deep_sleep();
    bsp_i2c_prepare_deep_sleep();

    // 3. 加锁等待当前 flush 完成，然后停止 ST7789 屏幕并保持 LCD/背光安全电平
    if (!bsp_lvgl_lock(1000)) {
        ESP_LOGE(TAG, "Failed to lock LVGL before deep sleep, restarting");
        esp_restart();
    }
    bsp_display_prepare_deep_sleep();

    // 4. 将 GPIO0（三键公共 ADC 节点）配置为数字输入
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << GPIO_NUM_0,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    // 5. 启用 GPIO0 低电平唤醒（按任意键时 GPIO0 被拉低唤醒系统）
    esp_err_t err = esp_deep_sleep_enable_gpio_wakeup(1ULL << GPIO_NUM_0, ESP_GPIO_WAKEUP_GPIO_LOW);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable GPIO wakeup: %s", esp_err_to_name(err));
    }

    ESP_LOGI(TAG, "Deep sleep armed; press any button to wake");
    esp_deep_sleep_start();

    // 理论上不会运行到此处
    ESP_LOGE(TAG, "esp_deep_sleep_start returned unexpectedly, restarting");
    esp_restart();
}

static void on_button(bsp_btn_t button, bsp_btn_ev_t event, void *user)
{
    (void)user;
    update_activity();
    if (!s_event_queue) return;

    app_event_t app_ev;
    bool send = false;

    if (button == BSP_BTN_UP && event == BSP_BTN_PRESS) {
        app_ev = APP_EVENT_DRAW_SINGLE;
        send = true;
    } else if (button == BSP_BTN_DOWN && event == BSP_BTN_PRESS) {
        app_ev = APP_EVENT_DRAW_TEN;
        send = true;
    } else if (button == BSP_BTN_OK) {
        if (event == BSP_BTN_CLICK) {
            app_ev = APP_EVENT_TOGGLE_MODE;
            send = true;
        } else if (event == BSP_BTN_LONG) {
            app_ev = APP_EVENT_RESET;
            send = true;
        }
    }

    if (send) {
        xQueueSendFromISR(s_event_queue, &app_ev, NULL);
    }
}

static void app_worker_task(void *arg)
{
    (void)arg;
    app_event_t ev;

    while (1) {
        if (xQueueReceive(s_event_queue, &ev, pdMS_TO_TICKS(1000)) == pdTRUE) {
            update_activity();
            switch (ev) {
                case APP_EVENT_DRAW_SINGLE: {
                    tianshang_draw_batch_t batch;
                    tianshang_draw(&s_state, 1, &batch);

                    if (batch.tian_count > 0) {
                        tianshang_audio_play(TIANSHANG_SFX_TIAN);
                    }

                    if (bsp_lvgl_lock(200)) {
                        tianshang_ui_on_single_draw(&batch.items[0], &s_state);
                        bsp_lvgl_unlock();
                    }
                    persist_state();
                    break;
                }

                case APP_EVENT_DRAW_TEN: {
                    tianshang_draw_batch_t batch;
                    tianshang_draw(&s_state, 10, &batch);

                    if (batch.tian_count > 0) {
                        tianshang_audio_play(TIANSHANG_SFX_TIAN);
                    }

                    if (bsp_lvgl_lock(200)) {
                        tianshang_ui_on_ten_draw(&batch, &s_state);
                        bsp_lvgl_unlock();
                    }
                    persist_state();
                    break;
                }

                case APP_EVENT_TOGGLE_MODE: {
                    if (bsp_lvgl_lock(200)) {
                        if (tianshang_ui_get_view() == UI_VIEW_RESULT) {
                            tianshang_ui_close_result(&s_state);
                        } else {
                            tianshang_ui_toggle_stats(&s_state);
                        }
                        bsp_lvgl_unlock();
                    }
                    break;
                }

                case APP_EVENT_RESET: {
                    tianshang_init_state(&s_state);
                    if (bsp_lvgl_lock(200)) {
                        tianshang_ui_reset(&s_state);
                        bsp_lvgl_unlock();
                    }
                    // 清空必须一并抹掉落盘记录，否则下次开机旧记录会被读回来。
                    if (tianshang_store_clear() != TIANSHANG_STORE_OK) {
                        ESP_LOGE(TAG, "Failed to clear stored draw state");
                    }
                    ESP_LOGI(TAG, "Simulation statistics reset");
                    break;
                }

                default:
                    break;
            }
        } else {
            // 每秒轮询超时检查：30 秒降亮，1 分钟深度休眠
            int64_t now = esp_timer_get_time();
            int64_t idle_us = now - s_last_activity_us;
            if (idle_us >= IDLE_SLEEP_TIMEOUT_US) {
                enter_deep_sleep();
            } else if (idle_us >= IDLE_DIM_TIMEOUT_US && !s_is_dimmed) {
                s_is_dimmed = true;
                bsp_display_backlight(BACKLIGHT_DIM);
                ESP_LOGI(TAG, "Inactivity reached 30s; dimming backlight to %d%%", BACKLIGHT_DIM);
            }
        }
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "FoloToy AI Passport Tianshang Gacha starting");

    esp_sleep_wakeup_cause_t wakeup_cause = esp_sleep_get_wakeup_cause();
    if (wakeup_cause == ESP_SLEEP_WAKEUP_GPIO) {
        ESP_LOGI(TAG, "Woke up from deep sleep by button press");
    }

    // 初始化活跃时间戳
    update_activity();

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // Initialize simulation state, then restore the stored records if any.
    // A missing or damaged record starts a fresh run instead of failing startup.
    tianshang_init_state(&s_state);
    switch (tianshang_store_load(&s_state)) {
        case TIANSHANG_STORE_OK:
            ESP_LOGI(TAG, "Restored %lu draws from NVS",
                     (unsigned long)s_state.total_draw_count);
            break;
        case TIANSHANG_STORE_NOT_FOUND:
            ESP_LOGI(TAG, "No stored records; starting a new run");
            break;
        case TIANSHANG_STORE_INVALID:
            ESP_LOGW(TAG, "Stored records are invalid; starting a new run");
            break;
        default:
            ESP_LOGE(TAG, "Failed to read stored records; starting a new run");
            break;
    }

    // Initialize Display & LVGL first for instant screen rendering
    if (bsp_display_init() != ESP_OK || !bsp_lvgl_init()) {
        ESP_LOGE(TAG, "Display init failed (MOSI=%d SCLK=%d CS=%d DC=%d BL=%d)",
                 BSP_LCD_MOSI, BSP_LCD_SCLK, BSP_LCD_CS, BSP_LCD_DC, BSP_LCD_BL);
        return;
    }
    bsp_display_backlight(BACKLIGHT_NORMAL);

    // Initialize UI
    if (!bsp_lvgl_lock(1000)) {
        ESP_LOGE(TAG, "LVGL lock timeout");
        return;
    }
    tianshang_ui_init(&s_state);
    bsp_lvgl_unlock();

    // Initialize I2C and battery fuel gauge
    bsp_i2c_init();
    bsp_battery_init();

    int initial_soc = bsp_battery_soc();
    if (bsp_lvgl_lock(200)) {
        tianshang_ui_update_battery(initial_soc);
        // Create 5-second periodic battery update timer in LVGL
        s_battery_timer = lv_timer_create(battery_timer_cb, 5000, NULL);
        (void)s_battery_timer;
        bsp_lvgl_unlock();
    }

    // Initialize audio effects
    bool audio_ok = tianshang_audio_init();
    ESP_LOGI(TAG, "Audio subsystem ready: %d", audio_ok);

    // Create event queue and worker task
    s_event_queue = xQueueCreate(16, sizeof(app_event_t));
    if (!s_event_queue) {
        ESP_LOGE(TAG, "Failed to create event queue");
        return;
    }

    // 6144 而非 4096:该任务除 UI 刷新外还要在抽奖后写 NVS，
    // flash 写入路径的栈开销与 LVGL 文本渲染叠加后，4KB 余量偏紧。
    if (xTaskCreate(app_worker_task, "ts_worker", 6144, NULL, 5, NULL) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create worker task");
        return;
    }

    // Initialize buttons
    if (bsp_button_init(on_button, NULL) != ESP_OK) {
        ESP_LOGE(TAG, "Button init failed");
        return;
    }

    ESP_LOGI(TAG, "Tianshang Gacha ready: UP=Single Pull, DOWN=Ten Pull, OK=Stats/Reset");
}
