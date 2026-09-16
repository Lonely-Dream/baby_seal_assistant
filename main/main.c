#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ui.h"
#include "can.h"
#include "vehicle_info.h"

#define LOG_TAG "main"

void Task100ms(void* arg)
{
    TickType_t xLastWakeTime;
    BaseType_t xWasDelayed;

    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();
    for (;;) {
        /// @todo add custom tasks here

        // Wait for the next cycle.
        xWasDelayed = xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));

        // Perform action here. xWasDelayed value can be used to determine
        // whether a deadline was missed if the code here took too long.
        (void)xWasDelayed;
    }
}

void app_main(void)
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

    os_ret = xTaskCreate(
        Task100ms,              // 任务函数
        "Task100ms",            // 任务名称
        2048,                   // 任务堆栈大小
        NULL,                   // 任务参数
        tskIDLE_PRIORITY + 2,   // 任务优先级
        NULL                    // 任务句柄
    );
    if (os_ret != pdPASS) {
        ESP_LOGE(LOG_TAG, "Failed to create Task100ms task");
        return;
    }

    // 10ms task loop
    TickType_t xLastWakeTime;
    BaseType_t xWasDelayed;

    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();
    for (;;) {
        /// @todo add custom 10ms tasks here
        StepCan();
        StepUi();

        // Wait for the next cycle.
        xWasDelayed = xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10));

        // Perform action here. xWasDelayed value can be used to determine
        // whether a deadline was missed if the code here took too long.
        (void)xWasDelayed;
    }
}
