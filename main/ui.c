#include "ui.h"

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lv_adapter.h"
#include "esp_log.h"
#include "ws_lcd_4.3B.h"
#include "vehicle_info.h"
#include "utils.h"

#define LOG_TAG "ui"

#define ANIM_DURATION_MS 1000

struct MeterConfig {
    int32_t x;
    int32_t y;
    int32_t size;
    int32_t init_value;
    int32_t min_value;
    int32_t max_value;
    uint32_t major_tick;
    uint32_t minor_tick;
    /// @brief 动画持续时间
    /// @note 单位: 毫秒
    uint32_t anim_duration;
    lv_anim_exec_xcb_t exec_cb;
    const char* unit;
};
typedef struct MeterConfig MeterConfig;

struct Meter {
    lv_obj_t* scale;
    lv_obj_t* line;
    lv_obj_t* label;
    lv_obj_t* label_unit;
    lv_anim_t anim;

    MeterConfig cfg;
    int32_t value;
    int32_t range;
    uint32_t line_length;
};
typedef struct Meter Meter;

void MeterInit(Meter* meter)
{
    static lv_style_t style_scale_items;
    static lv_style_t style_scale_indicator;
    static lv_style_t style_label;
    static lv_style_t style_label_unit;
    static lv_style_t style_line_main;
    static const lv_color_t text_color = {
        .red = 0xFF,
        .green = 0x80,
        .blue = 0x00
    };
    static bool style_inited = false;
    if (!style_inited) {
        /*Init all styles*/
        lv_style_init(&style_scale_items);
        lv_style_init(&style_scale_indicator);
        lv_style_init(&style_label);
        lv_style_init(&style_line_main);

        // 次刻度样式
        lv_style_set_length(&style_scale_items, 5);
        // 主刻度样式
        lv_style_set_length(&style_scale_indicator, 10);
        lv_style_set_line_width(&style_scale_indicator, 3);
        lv_style_set_line_color(&style_scale_indicator, lv_color_hex(0xFF0000));
        lv_style_set_pad_radial(&style_scale_indicator, 15);
        lv_style_set_text_color(&style_scale_indicator, text_color);
        lv_style_set_text_font(&style_scale_indicator, &lv_font_montserrat_28);
        // 单位标签样式(子级)
        lv_style_set_align(&style_label_unit, LV_ALIGN_BOTTOM_MID);
        lv_style_set_text_font(&style_label_unit, &lv_font_montserrat_16);
        // 数值标签样式(父级)
        lv_style_set_align(&style_label, LV_ALIGN_CENTER);
        lv_style_set_y(&style_label, 100);
        lv_style_set_text_color(&style_label, text_color);
        lv_style_set_text_align(&style_label, LV_TEXT_ALIGN_CENTER);
        lv_style_set_text_font(&style_label, &lv_font_montserrat_48);
        lv_style_set_height(&style_label, 16 + 48);
        // 指针样式
        lv_style_set_line_width(&style_line_main, 4);
        lv_style_set_line_rounded(&style_line_main, true);
        style_inited = true;
    }

    // 创建刻度
    meter->scale = lv_scale_create(lv_screen_active());
    lv_obj_set_size(meter->scale, meter->cfg.size, meter->cfg.size);
    lv_obj_set_pos(meter->scale, meter->cfg.x, meter->cfg.y);
    lv_obj_add_style(meter->scale, &style_scale_items, LV_PART_ITEMS);
    lv_obj_add_style(meter->scale, &style_scale_indicator, LV_PART_INDICATOR);

    // 设置刻度为圆弧样式
    lv_scale_set_mode(meter->scale, LV_SCALE_MODE_ROUND_INNER);
    lv_scale_set_min_value(meter->scale, meter->cfg.min_value);
    lv_scale_set_max_value(meter->scale, meter->cfg.max_value);
    meter->range = meter->cfg.max_value - meter->cfg.min_value;
    lv_scale_set_total_tick_count(meter->scale, meter->range / meter->cfg.minor_tick + 1);
    lv_scale_set_major_tick_every(meter->scale, meter->cfg.major_tick / meter->cfg.minor_tick);

    // 创建指针
    meter->line = lv_line_create(meter->scale);
    lv_obj_add_style(meter->line, &style_line_main, LV_PART_MAIN);

    lv_anim_init(&meter->anim);
    lv_anim_set_var(&meter->anim, meter); // 设置要应用动画的组件
    lv_anim_set_exec_cb(&meter->anim, meter->cfg.exec_cb);
    lv_anim_set_duration(&meter->anim, meter->cfg.anim_duration); // 动画持续时间ms
    lv_anim_set_repeat_count(&meter->anim, 1);

    // 创建标签
    meter->label = lv_label_create(meter->scale);
    lv_obj_add_style(meter->label, &style_label, LV_PART_MAIN);
    meter->label_unit = lv_label_create(meter->label);
    lv_obj_add_style(meter->label_unit, &style_label_unit, LV_PART_MAIN);

    meter->value = meter->cfg.init_value;
    // meter->line_length = 1000 * meter->cfg.size / 2 / 1618;
    meter->line_length = meter->cfg.size / 2 - 10 - 5;
    // 初始化指针位置和标签显示
    lv_scale_set_line_needle_value(meter->scale, meter->line, meter->line_length, meter->value);
    lv_label_set_text(meter->label, "--");
    lv_label_set_text(meter->label_unit, meter->cfg.unit);
}

void MeterSetValue(Meter* meter, int32_t value)
{
    if (value == meter->value) {
        return;
    }
    if (value < meter->cfg.min_value) {
        value = meter->cfg.min_value;
    } else if (value > meter->cfg.max_value) {
        value = meter->cfg.max_value;
    }
    lv_anim_set_values(&meter->anim, meter->value, value);
    lv_anim_start(&meter->anim);
    meter->value = value;
}

void MeterAnimCallback(void* obj, int32_t value)
{
    Meter* meter = (Meter*)obj;
    lv_scale_set_line_needle_value(meter->scale, meter->line, meter->line_length, value);
    lv_label_set_text_fmt(meter->label, "%" PRId32, value);
}

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
    esp_err_t esp_ret = InitWsLcd(num_fbs, &panel_handle);
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
    lv_display_t* disp = esp_lv_adapter_register_display(&disp_cfg);
    assert(disp != NULL);


    MeterInit(&meter_speed);
    MeterInit(&meter_power);
    ESP_ERROR_CHECK(esp_lv_adapter_start());
    return ESP_OK;
}
