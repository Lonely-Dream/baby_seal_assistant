#include "ws_lcd_4.3B.h"

#include "esp_log.h"
#include "driver/gpio.h"

static const char* TAG = "ws_lcd_4.3B";

#define LCD_PIXEL_CLOCK_HZ      (16 * 1000 * 1000)

#define RGB_DATA_WIDTH          (16)
#define RGB_BOUNCE_LINES        (10)
#define RGB_BOUNCE_BUFFER_SIZE  (LCD_H_RES * RGB_BOUNCE_LINES)

#define LCD_DMA_BURST_SIZE       (64)

// IO pin definitions for the RGB LCD panel
#define LCD_IO_RGB_DISP         (-1)
#define LCD_IO_RGB_VSYNC        (GPIO_NUM_3)
#define LCD_IO_RGB_HSYNC        (GPIO_NUM_46)
#define LCD_IO_RGB_DE           (GPIO_NUM_5)
#define LCD_IO_RGB_PCLK         (GPIO_NUM_7)
#define LCD_IO_RGB_DATA0        (GPIO_NUM_14)
#define LCD_IO_RGB_DATA1        (GPIO_NUM_38)
#define LCD_IO_RGB_DATA2        (GPIO_NUM_18)
#define LCD_IO_RGB_DATA3        (GPIO_NUM_17)
#define LCD_IO_RGB_DATA4        (GPIO_NUM_10)
#define LCD_IO_RGB_DATA5        (GPIO_NUM_39)
#define LCD_IO_RGB_DATA6        (GPIO_NUM_0)
#define LCD_IO_RGB_DATA7        (GPIO_NUM_45)
#define LCD_IO_RGB_DATA8        (GPIO_NUM_48)
#define LCD_IO_RGB_DATA9        (GPIO_NUM_47)
#define LCD_IO_RGB_DATA10       (GPIO_NUM_21)
#define LCD_IO_RGB_DATA11       (GPIO_NUM_1)
#define LCD_IO_RGB_DATA12       (GPIO_NUM_2)
#define LCD_IO_RGB_DATA13       (GPIO_NUM_42)
#define LCD_IO_RGB_DATA14       (GPIO_NUM_41)
#define LCD_IO_RGB_DATA15       (GPIO_NUM_40)


esp_err_t InitWsLcd(size_t num_fbs, esp_lcd_panel_handle_t* panel_handle)
{
    esp_err_t ret = ESP_OK;
    esp_lcd_rgb_panel_config_t panel_config = {
        .clk_src = LCD_CLK_SRC_DEFAULT, // Set the clock source for the panel
        .timings = {
            .pclk_hz = LCD_PIXEL_CLOCK_HZ, // Pixel clock frequency
            .h_res = LCD_H_RES,            // Horizontal resolution
            .v_res = LCD_V_RES,            // Vertical resolution
            .hsync_pulse_width = 4, // Horizontal sync pulse width
            .hsync_back_porch = 8,  // Horizontal back porch
            .hsync_front_porch = 8, // Horizontal front porch
            .vsync_pulse_width = 4, // Vertical sync pulse width
            .vsync_back_porch = 8,  // Vertical back porch
            .vsync_front_porch = 8, // Vertical front porch
            .flags = {
                .pclk_active_neg = 1, // Active low pixel clock
            },
        },
        .data_width = RGB_DATA_WIDTH,                       // Data width for RGB
        .in_color_format = LCD_COLOR_FMT_RGB565,            // Input color format
        .out_color_format = LCD_COLOR_FMT_RGB565,           // Output color format
        .num_fbs = num_fbs,                                  // Number of frame buffers
        .user_fbs = {NULL,NULL,NULL},// User-provided frame buffers (none in this case)
        .bounce_buffer_size_px = RGB_BOUNCE_BUFFER_SIZE,    // Bounce buffer size in pixels
        .dma_burst_size = LCD_DMA_BURST_SIZE, // DMA burst size
        .hsync_gpio_num = LCD_IO_RGB_HSYNC,                 // GPIO number for horizontal sync
        .vsync_gpio_num = LCD_IO_RGB_VSYNC,                 // GPIO number for vertical sync
        .de_gpio_num = LCD_IO_RGB_DE,                       // GPIO number for data enable
        .pclk_gpio_num = LCD_IO_RGB_PCLK,                   // GPIO number for pixel clock
        .disp_gpio_num = LCD_IO_RGB_DISP,                   // GPIO number for display
        .data_gpio_nums = {
            LCD_IO_RGB_DATA0,
            LCD_IO_RGB_DATA1,
            LCD_IO_RGB_DATA2,
            LCD_IO_RGB_DATA3,
            LCD_IO_RGB_DATA4,
            LCD_IO_RGB_DATA5,
            LCD_IO_RGB_DATA6,
            LCD_IO_RGB_DATA7,
            LCD_IO_RGB_DATA8,
            LCD_IO_RGB_DATA9,
            LCD_IO_RGB_DATA10,
            LCD_IO_RGB_DATA11,
            LCD_IO_RGB_DATA12,
            LCD_IO_RGB_DATA13,
            LCD_IO_RGB_DATA14,
            LCD_IO_RGB_DATA15,
        },
        .flags = {
            .fb_in_psram = true, // Use PSRAM for framebuffer
        },
    };

    ret = esp_lcd_new_rgb_panel(&panel_config, panel_handle);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "esp_lcd_new_rgb_panel ok");
    } else {
        *panel_handle = NULL;
        ESP_LOGE(TAG, "esp_lcd_new_rgb_panel failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_lcd_panel_init(*panel_handle);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "esp_lcd_panel_init ok");
    } else {
        ESP_LOGE(TAG, "esp_lcd_panel_init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    return ESP_OK;
}



