#include "speed_page.h"

SpeedPage::SpeedPage()
    : VehicleMeterPage(
        PAGE_ID,
        VehicleMeterPageConfig{ 160, 0, 480, 0, 0, 160, 20, 5, "km/h" },
        VEHICLE_INFO_VALID_IC_VEHICLE_SPEED)
{ }

int32_t SpeedPage::GetVehicleValue(const VehicleInfo& info) const
{
    return (int32_t)info.ic_veh_spd;
}