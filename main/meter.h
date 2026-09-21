#ifndef METER_H_
#define METER_H_

#include <stdint.h>
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

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

    void MeterInit(Meter* meter, lv_obj_t* parent);

    void MeterSetValue(Meter* meter, int32_t value);

#ifdef __cplusplus
}
#endif

#endif // METER_H_