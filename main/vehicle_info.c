#include "vehicle_info.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static VehicleInfo g_vehicle_info;
static portMUX_TYPE g_vehicle_info_lock = portMUX_INITIALIZER_UNLOCKED;

static Msg3D9 g_msg_3d9;

static uint32_t GetUpdateTimeMs(void)
{
    return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

void MockVehicleInfoReceive(const VehicleInfo* info)
{
    portENTER_CRITICAL(&g_vehicle_info_lock);
    g_vehicle_info = *info;
    portEXIT_CRITICAL(&g_vehicle_info_lock);
}

bool IsRequiredMessage(uint32_t id)
{
    switch (id) {
    case 0x238:
    case 0x3D9:
        return true;
    default:
        return false;
    }
}

void VehicleInfoReceiveCan(const CanMessage* msg)
{
    if (msg == NULL) {
        return;
    }

    const uint32_t updated_at_ms = GetUpdateTimeMs();
    switch (msg->id) {
    case 0x238: {
        const Msg238* msg_238 = (const Msg238*)msg->data;
        const uint32_t eng_spd = msg_238->EngSpd * 0.25;
        const uint32_t veh_spd = msg_238->VehSpd * 0.0625;

        portENTER_CRITICAL(&g_vehicle_info_lock);
        g_vehicle_info.eng_spd = eng_spd;
        g_vehicle_info.veh_spd = veh_spd;
        g_vehicle_info.valid_mask |= VEHICLE_INFO_VALID_ENGINE_SPEED | VEHICLE_INFO_VALID_VEHICLE_SPEED;
        g_vehicle_info.updated_at_ms = updated_at_ms;
        portEXIT_CRITICAL(&g_vehicle_info_lock);
        break;
    }
    case 0x3D9: {
        uint8_t i = msg->data[0];
        if (i < 1 || i > 7) {
            break;
        }
        memcpy(g_msg_3d9.buffer + (i - 1) * 7, msg->data + 1, 7);
        if (i == 4) {
            const int32_t power = (int32_t)(g_msg_3d9.Power * 0.5 - 1000.5);
            portENTER_CRITICAL(&g_vehicle_info_lock);
            g_vehicle_info.power = power;
            g_vehicle_info.valid_mask |= VEHICLE_INFO_VALID_POWER;
            g_vehicle_info.updated_at_ms = updated_at_ms;
            portEXIT_CRITICAL(&g_vehicle_info_lock);
        } else if (i == 6) {
            const uint32_t ic_veh_spd = g_msg_3d9.IcVehSpd;
            portENTER_CRITICAL(&g_vehicle_info_lock);
            g_vehicle_info.ic_veh_spd = ic_veh_spd;
            g_vehicle_info.valid_mask |= VEHICLE_INFO_VALID_IC_VEHICLE_SPEED;
            g_vehicle_info.updated_at_ms = updated_at_ms;
            portEXIT_CRITICAL(&g_vehicle_info_lock);
        }
        break;
    }
    default: {
        break;
    }
    }
}

bool VehicleInfoGetSnapshot(VehicleInfo* snapshot)
{
    if (snapshot == NULL) {
        return false;
    }

    portENTER_CRITICAL(&g_vehicle_info_lock);
    *snapshot = g_vehicle_info;
    portEXIT_CRITICAL(&g_vehicle_info_lock);
    return true;
}

esp_err_t InitVehicleInfo()
{
    memset(&g_msg_3d9, 0, sizeof(g_msg_3d9));
    portENTER_CRITICAL(&g_vehicle_info_lock);
    memset(&g_vehicle_info, 0, sizeof(g_vehicle_info));
    portEXIT_CRITICAL(&g_vehicle_info_lock);
    return ESP_OK;
}