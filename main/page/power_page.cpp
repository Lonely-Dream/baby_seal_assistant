#include "power_page.h"

PowerPage::PowerPage()
    : VehicleMeterPage(
        PAGE_ID,
        VehicleMeterPageConfig{
            .x = 160, .y = 25,
            .size = 480,
            .init_value = 0, .min_value = -60, .max_value = 100,
            .major_tick = 20, .minor_tick = 5,
            .unit = "kW\nPower",
            .custom_tick_label = nullptr,
        },
        VEHICLE_INFO_VALID_POWER)
{ }

int32_t PowerPage::GetVehicleValue(const VehicleInfo& info) const
{
    return info.power;
}