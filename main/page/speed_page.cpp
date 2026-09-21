#include "speed_page.h"

SpeedPage::SpeedPage()
    : VehicleMeterPage(
        PAGE_ID,
        VehicleMeterPageConfig{
            .x = 160, .y = 25,
            .size = 480, .init_value = 0, .min_value = 0, .max_value = 160,
            .major_tick = 20, .minor_tick = 5,
            .unit = "km/h\nSpeed",
            .custom_tick_label = nullptr,
        },
        VEHICLE_INFO_VALID_IC_VEHICLE_SPEED)
{ }

int32_t SpeedPage::GetVehicleValue(const VehicleInfo& info) const
{
    return (int32_t)info.ic_veh_spd;
}