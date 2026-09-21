#pragma once

#include "vehicle_meter_page.h"

class PowerPage final : public VehicleMeterPage {
public:
    static constexpr const char* PAGE_ID = "PowerPage";
    PowerPage();

protected:
    int32_t GetVehicleValue(const VehicleInfo& info) const override;
};