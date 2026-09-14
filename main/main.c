#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ws_lcd_4.3B.h"
#include "esp_lv_adapter.h"
#include "can.h"
#include "vehicle_info.h"

static const char* LOG_TAG = "main";
#define METER_UPDATE_INTERVAL_MS 200
#define ANIM_DURATION_MS METER_UPDATE_INTERVAL_MS

struct MeterConfig {
    int32_t x;
    int32_t y;
    int32_t size;
    int32_t min_value;
    int32_t max_value;
    uint32_t major_tick;
    uint32_t minor_tick;
    uint32_t anim_duration;
    lv_anim_exec_xcb_t exec_cb;
    const char* label_fmt;
};
typedef struct MeterConfig MeterConfig;

struct Meter {
    lv_obj_t* scale;
    lv_obj_t* line;
    lv_obj_t* arc;
    lv_obj_t* label;
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
    static bool style_inited = false;
    if (!style_inited) {
        /*Init all styles*/
        lv_style_init(&style_scale_items);
        lv_style_init(&style_scale_indicator);

        lv_style_set_length(&style_scale_items, 5);
        lv_style_set_line_color(&style_scale_items, lv_color_hex(0xFF8000));
        lv_style_set_length(&style_scale_indicator, 10);
        lv_style_set_line_width(&style_scale_indicator, 3);
        lv_style_set_line_color(&style_scale_indicator, lv_color_hex(0xFF0000));

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
    lv_obj_set_style_line_width(meter->line, 4, LV_PART_MAIN);
    lv_obj_set_style_line_rounded(meter->line, true, LV_PART_MAIN);

    lv_anim_init(&meter->anim);
    lv_anim_set_var(&meter->anim, meter); // 设置要应用动画的组件
    lv_anim_set_exec_cb(&meter->anim, meter->cfg.exec_cb);
    lv_anim_set_duration(&meter->anim, meter->cfg.anim_duration); // 动画持续时间ms
    lv_anim_set_repeat_count(&meter->anim, 1);

    // 创建标签
    meter->label = lv_label_create(meter->scale);
    lv_obj_set_align(meter->label, LV_ALIGN_CENTER);
    lv_obj_set_y(meter->label, 50);

    // 创建圆弧
    // lv_obj_t* arc = lv_arc_create(scale);
    // lv_obj_set_size(arc,256,256);
    // lv_arc_set_min_value(arc,0);
    // lv_arc_set_max_value(arc,160);
    // lv_arc_set_value(arc, n);

    meter->value = 0;
    meter->line_length = 800 * meter->cfg.size / 2 / 1000;
}

void MeterSetValue(Meter* meter, int32_t value)
{
    lv_anim_set_values(&meter->anim, meter->value, value);
    lv_anim_start(&meter->anim);
    meter->value = value;
}

void MeterAnimCallback(void* obj, int32_t value)
{
    Meter* meter = (Meter*)obj;
    lv_scale_set_line_needle_value(meter->scale, meter->line, meter->line_length, value);
    lv_label_set_text_fmt(meter->label, meter->cfg.label_fmt, value);
}

void app_main(void)
{
    esp_err_t esp_ret;
    BaseType_t os_ret;

    esp_ret = InitCan();
    if (esp_ret == ESP_OK) {
        ESP_LOGI(LOG_TAG, "CAN initialized");
    } else {
        ESP_LOGE(LOG_TAG, "CAN initialization failed: %s", esp_err_to_name(esp_ret));
    }
    os_ret = xTaskCreate(
        TaskCan,        // 任务函数
        "StepCanTask",  // 任务名称
        2048,           // 任务堆栈大小
        NULL,           // 任务参数
        5,              // 任务优先级
        NULL            // 任务句柄
    );
    if (os_ret != pdPASS) {
        ESP_LOGE(LOG_TAG, "Failed to create StepCan task");
        return;
    }

    uint8_t num_fbs = esp_lv_adapter_get_required_frame_buffer_count(
        ESP_LV_ADAPTER_TEAR_AVOID_MODE_DEFAULT_RGB, // 防撕裂模式
        ESP_LV_ADAPTER_ROTATE_0                     // 旋转角度
    );
    ESP_LOGI(LOG_TAG, "num_fbs=%hhu", num_fbs);
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_ret = InitWsLcd(num_fbs, &panel_handle);
    if (esp_ret == ESP_OK) {
        ESP_LOGI(LOG_TAG, "LCD initialized");
    } else {
        ESP_LOGE(LOG_TAG, "LCD initialization failed: %s", esp_err_to_name(esp_ret));
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

    Meter meter_speed = {
        .cfg = {
            .x = 6,
            .y = LCD_V_RES / 2 - 390 / 2,
            .size = 390,
            .min_value = 0,
            .max_value = 160,
            .major_tick = 10,
            .minor_tick = 5,
            .anim_duration = ANIM_DURATION_MS,
            .exec_cb = MeterAnimCallback,
            .label_fmt = "Speed %d km/h",
        } };
    Meter meter_power = {
        .cfg = {
            .x = 400 + 8 / 2,
            .y = LCD_V_RES / 2 - 390 / 2,
            .size = 390,
            .min_value = -60,
            .max_value = 120,
            .major_tick = 10,
            .minor_tick = 5,
            .anim_duration = ANIM_DURATION_MS,
            .exec_cb = MeterAnimCallback,
            .label_fmt = "Power %d kw",
        } };
    MeterInit(&meter_speed);
    MeterInit(&meter_power);
    ESP_ERROR_CHECK(esp_lv_adapter_start());

    while (true) {
        if (esp_lv_adapter_lock(-1) == ESP_OK) {
            MeterSetValue(&meter_speed, g_vehicle_info.veh_spd);
            MeterSetValue(&meter_power, g_vehicle_info.power);
            esp_lv_adapter_unlock();
        }

        vTaskDelay(pdMS_TO_TICKS(METER_UPDATE_INTERVAL_MS));
    }
}
