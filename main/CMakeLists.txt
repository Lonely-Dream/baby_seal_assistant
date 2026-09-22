idf_component_register(
    SRCS
        "main.cpp"
        "ui.c"
        "meter.c"
        "can.c"
        "ws_lcd_4.3B.c"
        "vehicle_info.c"
        "page/vehicle_meter_page.cpp"
        "page/speed_page.cpp"
        "page/power_page.cpp"
        "page/heart_page.cpp"
    INCLUDE_DIRS "."
    PRIV_REQUIRES esp_lcd
    PRIV_REQUIRES esp_driver_twai
    PRIV_REQUIRES esp_driver_i2c
    PRIV_REQUIRES esp_driver_gpio
)
