#include "can.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_twai.h"
#include "esp_twai_onchip.h"
#include "vehicle_info.h"

#define LOG_TAG "can"

#define TX_GPIO_NUM GPIO_NUM_15
#define RX_GPIO_NUM GPIO_NUM_16
#define RX_QUEUE_DEPTH (16)

static const twai_onchip_node_config_t NODE_CFG = {
    .io_cfg.tx = GPIO_NUM_15,
    .io_cfg.rx = GPIO_NUM_16,
    .bit_timing.bitrate = 500000,
    .tx_queue_depth = 16,
};
static twai_node_handle_t g_node = NULL;
static QueueHandle_t g_rx_queue;

static bool OnCanRxDone(twai_node_handle_t handle, const twai_rx_done_event_data_t* edata, void* user_ctx)
{
    (void)edata;
    (void)user_ctx;
    esp_err_t ret = ESP_OK;
    BaseType_t xHigherPriorityTaskWoken = false;
    CanMessage msg;
    twai_frame_t rx_frame = {
        .buffer = msg.data,
        .buffer_len = sizeof(msg.data),
    };

    ret = twai_node_receive_from_isr(handle, &rx_frame);
    if (ret != ESP_OK) {
        return false;
    }
    if (!IsRequiredMessage(rx_frame.header.id)) {
        return false;
    }
    msg.id = rx_frame.header.id;
    msg.is_ext = rx_frame.header.ide;
    msg.dlc = rx_frame.header.dlc;
    msg.flag = 0xAA;
    if (xQueueSendFromISR(g_rx_queue, &msg, &xHigherPriorityTaskWoken) != pdPASS) {
        return false;
    }
    return xHigherPriorityTaskWoken == pdTRUE;
}

void ProcessCanTx()
{
    if (g_node == NULL) {
        return;
    }
    uint8_t buffer[8] = { 0 };
    twai_frame_t tx_frame = {
        .header.id = 0x18FFFFFF,
        .header.ide = 1,
        .header.dlc = sizeof(buffer),
        .buffer = buffer,
        .buffer_len = sizeof(buffer),
    };
    esp_err_t ret = twai_node_transmit(g_node, &tx_frame, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(LOG_TAG, "twai_node_transmit failed. %s", esp_err_to_name(ret));
    }
}

/// @brief 处理CAN任务
/// @note 10ms调度
void StepCan()
{
    CanMessage msg;
    if (g_rx_queue == NULL) {
        ESP_LOGE(LOG_TAG, "RX queue is NULL");
        return;
    }

    while (xQueueReceive(g_rx_queue, &msg, 0) == pdTRUE) {
        if (msg.flag != 0xAA) {
            continue;
        }
        VehicleInfoReceiveCan(&msg);
    }
}

esp_err_t InitCan()
{
    g_rx_queue = xQueueCreate(RX_QUEUE_DEPTH, sizeof(CanMessage));
    if (g_rx_queue == NULL) {
        ESP_LOGE(LOG_TAG, "Failed to create RX queue");
        return ESP_FAIL;
    }
    esp_err_t ret = ESP_OK;
    ret = twai_new_node_onchip(&NODE_CFG, &g_node);
    if (ret == ESP_OK) {
        ESP_LOGI(LOG_TAG, "twai_new_node_onchip ok");
    } else {
        ESP_LOGE(LOG_TAG, "twai_new_node_onchip failed");
        return ret;
    }
    static const twai_event_callbacks_t cbs = {
        .on_rx_done = OnCanRxDone,
    };
    ret = twai_node_register_event_callbacks(g_node, &cbs, NULL);
    if (ret == ESP_OK) {
        ESP_LOGI(LOG_TAG, "twai_node_register_event_callbacks ok");
    } else {
        ESP_LOGE(LOG_TAG, "twai_node_register_event_callbacks failed");
        return ret;
    }
    ret = twai_node_enable(g_node);
    if (ret == ESP_OK) {
        ESP_LOGI(LOG_TAG, "twai_node_enable ok");
    } else {
        ESP_LOGE(LOG_TAG, "twai_node_enable failed");
        return ret;
    }
    return ESP_OK;
}

esp_err_t DeinitCan()
{
    esp_err_t ret = ESP_OK;
    ret = twai_node_disable(g_node);
    if (ret == ESP_OK) {
        ESP_LOGI(LOG_TAG, "twai_node_disable ok");
    } else {
        ESP_LOGE(LOG_TAG, "twai_node_disable failed");
        return ESP_FAIL;
    }
    ret = twai_node_delete(g_node);
    if (ret == ESP_OK) {
        ESP_LOGI(LOG_TAG, "twai_node_delete ok");
    } else {
        ESP_LOGE(LOG_TAG, "twai_node_delete failed");
        return ESP_FAIL;
    }
    return ESP_OK;
}
