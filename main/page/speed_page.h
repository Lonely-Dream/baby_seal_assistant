#pragma once

#include "vehicle_meter_page.h"

class SpeedPage final : public VehicleMeterPage {
public:
    static constexpr const char* PAGE_ID = "SpeedPage";
    SpeedPage();

protected:
    int32_t GetVehicleValue(const VehicleInfo& info) const override;
};