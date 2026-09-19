#include "ui.h"

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lv_adapter.h"
#include "esp_log.h"
#include "ws_lcd_4.3B.h"
#include "vehicle_info.h"
#include "utils.h"
#include "meter.h"

#define LOG_TAG "ui"

#define ANIM_DURATION_MS 1000


void MeterAnimCallback(void* obj, int32_t value)
{
    Meter* meter = (Meter*)obj;
    lv_scale_set_line_needle_value(meter->scale, meter->line, meter->line_length, value);
    lv_label_set_text_fmt(meter->label, "%" PRId32, value);
}

lv_display_t* g_disp;
lv_indev_t* g_touch;
Meter meter_speed = {
    .cfg = {
        .x = 6,
        .y = LCD_V_RES / 2 - 390 / 2,
        .size = 390,
        .init_value = 0,
        .min_value = 0,
        .max_value = 160,
        .major_tick = 20,
        .minor_tick = 5,
        .anim_duration = ANIM_DURATION_MS,
        .exec_cb = MeterAnimCallback,
        .unit = "km/h",
    }
};
Meter meter_power = {
    .cfg = {
        .x = 400 + 8 / 2,
        .y = LCD_V_RES / 2 - 390 / 2,
        .size = 390,
        .init_value = 0,
        .min_value = -60,
        .max_value = 100,
        .major_tick = 20,
        .minor_tick = 5,
        .anim_duration = ANIM_DURATION_MS,
        .exec_cb = MeterAnimCallback,
        .unit = "kw",
    }
};
lv_obj_t* g_label_fps;

void SelfCheckUi()
{
    const TickType_t MAX_ANIM_DURATION = pdMS_TO_TICKS(MAX(
        meter_speed.cfg.anim_duration,
        meter_power.cfg.anim_duration));

    // Perform self-check for UI components
    if (esp_lv_adapter_lock(-1) == ESP_OK) {
        MeterSetValue(&meter_speed, meter_speed.cfg.max_value);
        MeterSetValue(&meter_power, meter_power.cfg.max_value);
        esp_lv_adapter_unlock();
    }
    vTaskDelay(MAX_ANIM_DURATION);

    if (esp_lv_adapter_lock(-1) == ESP_OK) {
        MeterSetValue(&meter_speed, meter_speed.cfg.min_value);
        MeterSetValue(&meter_power, meter_power.cfg.min_value);
        esp_lv_adapter_unlock();
    }
    vTaskDelay(MAX_ANIM_DURATION);

    if (esp_lv_adapter_lock(-1) == ESP_OK) {
        MeterSetValue(&meter_speed, meter_speed.cfg.init_value);
        MeterSetValue(&meter_power, meter_power.cfg.init_value);
        esp_lv_adapter_unlock();
    }
    vTaskDelay(MAX_ANIM_DURATION);
}

void StatUi()
{
    uint32_t fps = 0;
    esp_err_t esp_ret;
    esp_ret = esp_lv_adapter_get_fps(g_disp, &fps);
    if (esp_ret != ESP_OK) {
        ESP_LOGE(LOG_TAG, "esp_lv_adapter_get_fps failed: %s", esp_err_to_name(esp_ret));
        return;
    }
    lv_label_set_text_fmt(g_label_fps, "FPS:%" PRIu32, fps);
}

void StepUi()
{
    if (esp_lv_adapter_lock(-1) == ESP_OK) {
        MeterSetValue(&meter_speed, g_vehicle_info.ic_veh_spd);
        MeterSetValue(&meter_power, g_vehicle_info.power);
        esp_lv_adapter_unlock();
    }
}

esp_err_t InitUi()
{
    uint8_t num_fbs = esp_lv_adapter_get_required_frame_buffer_count(
        ESP_LV_ADAPTER_TEAR_AVOID_MODE_DEFAULT_RGB, // 防撕裂模式
        ESP_LV_ADAPTER_ROTATE_0                     // 旋转角度
    );
    ESP_LOGI(LOG_TAG, "num_fbs=%hhu", num_fbs);
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_touch_handle_t touch_handle = NULL;
    esp_err_t esp_ret = InitWsLcd(num_fbs, &panel_handle, &touch_handle);
    if (esp_ret == ESP_OK) {
        ESP_LOGI(LOG_TAG, "LCD initialized");
    } else {
        ESP_LOGE(LOG_TAG, "LCD initialization failed: %s", esp_err_to_name(esp_ret));
        return esp_ret;
    }

    esp_lv_adapter_config_t cfg = ESP_LV_ADAPTER_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(esp_lv_adapter_init(&cfg));

    esp_lv_adapter_display_config_t disp_cfg = ESP_LV_ADAPTER_DISPLAY_RGB_DEFAULT_CONFIG(
        panel_handle,           // LCD 面板句柄
        NULL,                   // LCD 面板 IO 句柄（某些接口可为 NULL）
        LCD_H_RES,              // 水平分辨率
        LCD_V_RES,              // 垂直分辨率
        ESP_LV_ADAPTER_ROTATE_0 // 旋转角度
    );
    g_disp = esp_lv_adapter_register_display(&disp_cfg);
    assert(g_disp != NULL);

    esp_lv_adapter_touch_config_t touch_cfg = ESP_LV_ADAPTER_TOUCH_DEFAULT_CONFIG(
        g_disp,
        touch_handle
    );
    g_touch = esp_lv_adapter_register_touch(&touch_cfg);
    assert(g_touch != NULL);

    g_label_fps = lv_label_create(lv_scr_act());
    lv_obj_set_align(g_label_fps, LV_ALIGN_TOP_RIGHT);

    MeterInit(&meter_speed);
    MeterInit(&meter_power);
    ESP_ERROR_CHECK(esp_lv_adapter_start());

    esp_ret = esp_lv_adapter_fps_stats_enable(g_disp, true);
    if (esp_ret == ESP_OK) {
        ESP_LOGI(LOG_TAG, "esp_lv_adapter_fps_stats_enable ok");
    } else {
        ESP_LOGE(LOG_TAG, "esp_lv_adapter_fps_stats_enable failed: %s", esp_err_to_name(esp_ret));
    }
    return ESP_OK;
}
