idf_component_register(
    SRCS "ui.c" "meter.c" "can.c" "main.c" "ws_lcd_4.3B.c" "vehicle_info.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES esp_lcd
    PRIV_REQUIRES esp_driver_twai
    PRIV_REQUIRES esp_driver_i2c
    PRIV_REQUIRES esp_driver_gpio
)
