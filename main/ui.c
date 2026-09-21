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

lv_display_t* g_disp;
lv_indev_t* g_touch;

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

    ESP_ERROR_CHECK(esp_lv_adapter_start());
    return ESP_OK;
}
