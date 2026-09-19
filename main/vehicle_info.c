#include "vehicle_info.h"

#include <string.h>

VehicleInfo g_vehicle_info;

static Msg3D9 g_msg_3d9;

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
    switch (msg->id) {
    case 0x238: {
        Msg238* msg_238 = (Msg238*)msg->data;
        g_vehicle_info.eng_spd = msg_238->EngSpd * 0.25;
        g_vehicle_info.veh_spd = msg_238->VehSpd * 0.0625;
        break;
    }
    case 0x3D9: {
        uint8_t i = msg->data[0];
        if (i < 1 || i > 7) {
            break;
        }
        memcpy(g_msg_3d9.buffer + (i - 1) * 7, msg->data + 1, 7);
        g_vehicle_info.power = g_msg_3d9.Power * 0.5 - 1000.5;
        g_vehicle_info.ic_veh_spd = g_msg_3d9.IcVehSpd;
        break;
    }
    default: {
        break;
    }
    }
}

esp_err_t InitVehicleInfo()
{
    g_vehicle_info.veh_spd = 0;
    g_vehicle_info.ic_veh_spd = 0;
    g_vehicle_info.eng_spd = 0;
    g_vehicle_info.power = 0;
    return ESP_OK;
}