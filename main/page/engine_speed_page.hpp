#pragma once

#include "vehicle_meter_page.h"

namespace {
    static const char* CUSTOM_TICK_LABEL[] = { "0","1","2","3","4","5","6","7","8",NULL };
}

class EngineSpeedPage final : public VehicleMeterPage {
public:
    static constexpr const char* PAGE_ID = "EngineSpeedPage";
    EngineSpeedPage()
        : VehicleMeterPage(
            PAGE_ID,
            VehicleMeterPageConfig{
                .x = 160, .y = 25,
                .size = 480, .init_value = 0, .min_value = 0, .max_value = 8000,
                .major_tick = 1000, .minor_tick = 250,
                .unit = "rpm\nEngine Speed",
                .custom_tick_label = CUSTOM_TICK_LABEL
            },
            VEHICLE_INFO_VALID_ENGINE_SPEED)
    {

    }

protected:
    int32_t GetVehicleValue(const VehicleInfo& info) const override
    {
        return info.eng_spd;
    }
};
