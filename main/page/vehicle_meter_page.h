#pragma once

#include <stdint.h>

#include "lvgl_nav_kit/page_base.h"
#include "meter.h"
#include "vehicle_info.h"

struct VehicleMeterPageConfig {
    int32_t x;
    int32_t y;
    int32_t size;
    int32_t init_value;
    int32_t min_value;
    int32_t max_value;
    uint32_t major_tick;
    uint32_t minor_tick;
    const char* unit;
    const char** custom_tick_label;
};

class VehicleMeterPage : public ui::PageBase {
public:
    VehicleMeterPage(const char* page_id, const VehicleMeterPageConfig& config, uint32_t valid_flag);

    void OnCreate(lv_obj_t* parent) override;
    void OnEnter() override;
    void OnDestroy() override;

protected:
    virtual int32_t GetVehicleValue(const VehicleInfo& info) const = 0;

private:
    void Refresh();
    static void RefreshTimer(lv_timer_t* timer);
    static void MeterAnimCallback(void* obj, int32_t value);

    VehicleMeterPageConfig config_;
    uint32_t valid_flag_;
    Meter meter_{ };
    lv_timer_t* refresh_timer_ = nullptr;
};