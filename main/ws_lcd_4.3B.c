#include "ws_lcd_4.3B.h"

#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_lcd_touch_gt911.h"

#define LOG_TAG "ws_lcd_4.3B"

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


#define I2C_MASTER_NUM          (I2C_NUM_0)
#define I2C_MASTER_SDA_IO       (GPIO_NUM_8)
#define I2C_MASTER_SCL_IO       (GPIO_NUM_9)
#define I2C_MASTER_FREQ_HZ      400000
#define I2C_ADDR_CH422G_MODE    (0x48>>1) // 0x24
#define I2C_ADDR_CH422G_IO_W    (0x70>>1) // 0x38
#define I2C_TIMEOUT_MS          (1000)

#define LCD_IO_TOUCH_RST        (-1)
#define LCD_IO_TOUCH_INT        (GPIO_NUM_4)

esp_err_t InitWsLcd(size_t num_fbs, esp_lcd_panel_handle_t* panel_handle, esp_lcd_touch_handle_t* touch_handle)
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
        .user_fbs = { NULL,NULL,NULL },// User-provided frame buffers (none in this case)
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
        ESP_LOGI(LOG_TAG, "esp_lcd_new_rgb_panel ok");
    } else {
        *panel_handle = NULL;
        ESP_LOGE(LOG_TAG, "esp_lcd_new_rgb_panel failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_lcd_panel_init(*panel_handle);
    if (ret == ESP_OK) {
        ESP_LOGI(LOG_TAG, "esp_lcd_panel_init ok");
    } else {
        ESP_LOGE(LOG_TAG, "esp_lcd_panel_init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags.enable_internal_pullup = true,// 使能内部上拉
        .flags.allow_pd = false, // 禁止睡眠模式下关闭外设电源
    };
    i2c_master_bus_handle_t bus_handle;
    ret = i2c_new_master_bus(&bus_config, &bus_handle);
    if (ret == ESP_OK) {
        ESP_LOGI(LOG_TAG, "i2c_new_master_bus ok");
    } else {
        ESP_LOGE(LOG_TAG, "i2c_new_master_bus failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // 创建面板io，内部会i2c_master_bus_add_device
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
    tp_io_config.scl_speed_hz = I2C_MASTER_FREQ_HZ;
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    ret = esp_lcd_new_panel_io_i2c(bus_handle, &tp_io_config, &tp_io_handle);
    if (ret == ESP_OK) {
        ESP_LOGI(LOG_TAG, "esp_lcd_new_panel_io_i2c ok");
    } else {
        ESP_LOGE(LOG_TAG, "esp_lcd_new_panel_io_i2c failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // IO口不够，这里需要通过CH422G扩展出来的EXIO1进行复位
    i2c_device_config_t dev_config_ch422g_mode = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = I2C_ADDR_CH422G_MODE,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    i2c_master_dev_handle_t dev_handle_ch422g_mode;
    ret = i2c_master_bus_add_device(bus_handle, &dev_config_ch422g_mode, &dev_handle_ch422g_mode);
    if (ret == ESP_OK) {
        ESP_LOGI(LOG_TAG, "i2c_master_bus_add_device ch422g_mode ok");
    } else {
        ESP_LOGE(LOG_TAG, "i2c_master_bus_add_device ch422g_mode failed: %s", esp_err_to_name(ret));
        return ret;
    }
    i2c_device_config_t dev_config_ch422g_io_w = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = I2C_ADDR_CH422G_IO_W,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    i2c_master_dev_handle_t dev_handle_ch422g_io_w;
    ret = i2c_master_bus_add_device(bus_handle, &dev_config_ch422g_io_w, &dev_handle_ch422g_io_w);
    if (ret == ESP_OK) {
        ESP_LOGI(LOG_TAG, "i2c_master_bus_add_device ch422g_io_w ok");
    } else {
        ESP_LOGE(LOG_TAG, "i2c_master_bus_add_device ch422g_io_w failed: %s", esp_err_to_name(ret));
        return ret;
    }
    // gpio_config_t io_config = {
    //     .pin_bit_mask = 1ULL << LCD_IO_TOUCH_INT,
    //     .mode = GPIO_MODE_OUTPUT,
    //     .pull_up_en = GPIO_PULLUP_DISABLE,
    //     .pull_down_en = GPIO_PULLDOWN_DISABLE,
    //     .intr_type = GPIO_INTR_DISABLE,
    // };
    ret = gpio_set_direction(LCD_IO_TOUCH_INT, GPIO_MODE_OUTPUT);
    if (ret == ESP_OK) {
        ESP_LOGI(LOG_TAG, "gpio_set_direction LCD_IO_TOUCH_INT ok");
    } else {
        ESP_LOGE(LOG_TAG, "gpio_set_direction LCD_IO_TOUCH_INT failed: %s", esp_err_to_name(ret));
        return ret;
    }

    gpio_set_level(LCD_IO_TOUCH_INT, 0);

    // Reset the touch controller via CH422G EXIO1 and GPIO
    uint8_t buffer = 0x01;
    i2c_master_transmit(dev_handle_ch422g_mode, &buffer, sizeof(buffer), I2C_TIMEOUT_MS);
    buffer = 0x2C; // RST = 0
    i2c_master_transmit(dev_handle_ch422g_io_w, &buffer, sizeof(buffer), I2C_TIMEOUT_MS);
    esp_rom_delay_us(100 * 1000);
    gpio_set_level(LCD_IO_TOUCH_INT, 0);// INT = 0
    esp_rom_delay_us(100 * 1000);
    buffer = 0x2E; // RST = 1
    i2c_master_transmit(dev_handle_ch422g_io_w, &buffer, sizeof(buffer), I2C_TIMEOUT_MS);
    esp_rom_delay_us(200 * 1000);

    esp_lcd_touch_io_gt911_config_t tp_gt911_config = {
        .dev_addr = ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS,
    };
    esp_lcd_touch_config_t tp_config = {
        .x_max = LCD_H_RES,
        .y_max = LCD_V_RES,
        .rst_gpio_num = LCD_IO_TOUCH_RST,
        .int_gpio_num = LCD_IO_TOUCH_INT,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
        .driver_data = &tp_gt911_config,
    };
    ret = esp_lcd_touch_new_i2c_gt911(tp_io_handle, &tp_config, touch_handle);
    if (ret == ESP_OK) {
        ESP_LOGI(LOG_TAG, "esp_lcd_touch_new_i2c_gt911 ok");
    } else {
        ESP_LOGE(LOG_TAG, "esp_lcd_touch_new_i2c_gt911 failed: %s", esp_err_to_name(ret));
        return ret;
    }
    return ESP_OK;
}
