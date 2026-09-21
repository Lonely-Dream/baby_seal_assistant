#include "power_page.h"

PowerPage::PowerPage()
    : VehicleMeterPage(
        PAGE_ID,
        VehicleMeterPageConfig{ 160, 0, 480, 0, -60, 100, 20, 5, "kW" },
        VEHICLE_INFO_VALID_POWER)
{ }

int32_t PowerPage::GetVehicleValue(const VehicleInfo& info) const
{
    return info.power;
}