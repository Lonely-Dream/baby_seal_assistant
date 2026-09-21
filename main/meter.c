#include "meter.h"


void MeterInit(Meter* meter, lv_obj_t* parent)
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
    meter->scale = lv_scale_create(parent);
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
