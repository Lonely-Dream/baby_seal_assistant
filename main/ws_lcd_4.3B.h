#pragma once

#include "esp_err.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_touch.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LCD_H_RES 800
#define LCD_V_RES 480

#define LCD_FRAME_BUFFER_SIZE    (LCD_H_RES * LCD_V_RES * sizeof(uint16_t))

    esp_err_t InitWsLcd(size_t num_fbs,
        esp_lcd_panel_handle_t* panel_handle,
        esp_lcd_touch_handle_t* touch_handle
    );

#ifdef __cplusplus
}
#endif
