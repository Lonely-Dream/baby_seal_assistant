#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_lv_adapter.h"
#include "ui.h"
#include "can.h"
#include "lvgl_nav_kit/ui_manager.h"
#include "lvgl_nav_kit/page_registry.h"
#include "page/speed_page.h"
#include "page/power_page.h"
#include "page/engine_speed_page.hpp"
#include "page/heart_page.h"
#include "esp_random.h"

#define LOG_TAG "main"

static void TaskCanRx(void* arg)
{
    (void)arg;
    TickType_t last_wake_time = xTaskGetTickCount();
    for (;;) {
        StepCan();
        xTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(10));
    }
}

static void MockVehicleInfo()
{
    VehicleInfo info;
    info.valid_mask = 0xFFFFFFFF;
    info.veh_spd = esp_random() % 160;
    info.ic_veh_spd = esp_random() % 160;
    info.eng_spd = esp_random() % 8000;
    info.power = (int32_t)(esp_random() % 160 - 60);
    MockVehicleInfoReceive(&info);
}

static void Task1000ms(void* arg)
{
    (void)arg;
    TickType_t last_wake_time = xTaskGetTickCount();
    for (;;) {
        ProcessCanTx();
        // MockVehicleInfo();
        xTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(1000));
    }
}

extern "C" void app_main()
{
    esp_err_t esp_ret;
    BaseType_t os_ret;
    // 初始化
    esp_ret = InitCan();
    if (esp_ret == ESP_OK) {
        ESP_LOGI(LOG_TAG, "CAN initialized");
    } else {
        ESP_LOGE(LOG_TAG, "CAN initialization failed: %s", esp_err_to_name(esp_ret));
        return;
    }
    esp_ret = InitVehicleInfo();
    if (esp_ret == ESP_OK) {
        ESP_LOGI(LOG_TAG, "Vehicle Info initialized");
    } else {
        ESP_LOGE(LOG_TAG, "Vehicle Info initialization failed: %s", esp_err_to_name(esp_ret));
        return;
    }
    esp_ret = InitUi();
    if (esp_ret == ESP_OK) {
        ESP_LOGI(LOG_TAG, "UI initialized");
    } else {
        ESP_LOGE(LOG_TAG, "UI initialization failed: %s", esp_err_to_name(esp_ret));
        return;
    }

    esp_ret = esp_lv_adapter_lock(-1);
    if (esp_ret != ESP_OK) {
        ESP_LOGE(LOG_TAG, "Failed to acquire LVGL lock: %s", esp_err_to_name(esp_ret));
        return;
    }

    auto& mgr = ui::UIManager::GetInstance();
    mgr.Initialize(lv_scr_act(), nullptr);
    mgr.SetTransitionDuration(500);
    mgr.SetMaxCachedPages(1);
    auto& reg = mgr.GetRegistry();
    auto* speed_page = new SpeedPage();
    auto* power_page = new PowerPage();
    auto* engine_speed_page = new EngineSpeedPage();
    auto* heart_page = new HeartPage();
    reg.RegisterPage(speed_page);
    reg.RegisterPage(power_page);
    reg.RegisterPage(engine_speed_page);
    reg.RegisterPage(heart_page);

    reg.SetNavigation(
        speed_page->PAGE_ID,
        ui::PageNavigation{
            .left = { power_page->PAGE_ID, ui::Direction::Left },
            .right = { engine_speed_page->PAGE_ID, ui::Direction::Right },
            .up = { },
            .down = { },
        });
    reg.SetNavigation(
        power_page->PAGE_ID,
        ui::PageNavigation{
            .left = { },
            .right = { speed_page->PAGE_ID, ui::Direction::Right },
            .up = { },
            .down = { },
        });
    reg.SetNavigation(
        engine_speed_page->PAGE_ID,
        ui::PageNavigation{
            .left = { speed_page->PAGE_ID, ui::Direction::Left },
            .right = { heart_page->PAGE_ID, ui::Direction::Right },
            .up = { },
            .down = { },
        });
    reg.SetNavigation(
        heart_page->PAGE_ID,
        ui::PageNavigation{
            .left = { engine_speed_page->PAGE_ID, ui::Direction::Left },
            .right = { },
            .up = { },
            .down = { },
        });

    mgr.NavigateTo(speed_page->PAGE_ID);
    esp_lv_adapter_unlock();

    os_ret = xTaskCreate(
        TaskCanRx,
        "TaskCanRx",
        2048,
        NULL,
        tskIDLE_PRIORITY + 3,
        NULL
    );
    if (os_ret != pdPASS) {
        ESP_LOGE(LOG_TAG, "Failed to create TaskCanRx task");
        return;
    }

    os_ret = xTaskCreate(
        Task1000ms,
        "Task1000ms",
        2048,
        NULL,
        tskIDLE_PRIORITY + 2,
        NULL
    );
    if (os_ret != pdPASS) {
        ESP_LOGE(LOG_TAG, "Failed to create Task1000ms task");
    }
}