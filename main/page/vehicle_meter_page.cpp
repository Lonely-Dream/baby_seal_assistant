#include "vehicle_meter_page.h"

#include <inttypes.h>

namespace {
    constexpr uint32_t kMeterPageBackgroundColor = 0x111416;
    constexpr uint32_t kMaxMeterAnimationDurationMs = 1500;
    constexpr uint32_t kMinMeterAnimationDurationMs = 100;
    constexpr uint32_t kVehicleRefreshPeriodMs = 30;
}

VehicleMeterPage::VehicleMeterPage(const char* page_id, const VehicleMeterPageConfig& config, uint32_t valid_flag)
    : ui::PageBase(page_id), config_(config), valid_flag_(valid_flag)
{ }

void VehicleMeterPage::OnCreate(lv_obj_t* parent)
{
    SetPageBackground(parent, kMeterPageBackgroundColor);
    meter_.cfg = MeterConfig{
        config_.x,
        config_.y,
        config_.size,
        config_.init_value,
        config_.min_value,
        config_.max_value,
        config_.major_tick,
        config_.minor_tick,
        kMinMeterAnimationDurationMs,
        kMaxMeterAnimationDurationMs,
        MeterAnimCallback,
        config_.unit,
        config_.custom_tick_label,
    };
    MeterInit(&meter_, parent);
}

void VehicleMeterPage::OnEnter()
{
    if (refresh_timer_ == nullptr) {
        refresh_timer_ = CreateTimer(RefreshTimer, kVehicleRefreshPeriodMs, this);
    }
    Refresh();
}

void VehicleMeterPage::OnDestroy()
{
    refresh_timer_ = nullptr;
    meter_ = { };
}

void VehicleMeterPage::Refresh()
{
    VehicleInfo info;
    if (!VehicleInfoGetSnapshot(&info) || (info.valid_mask & valid_flag_) == 0) {
        return;
    }
    MeterSetValue(&meter_, GetVehicleValue(info));
}

void VehicleMeterPage::RefreshTimer(lv_timer_t* timer)
{
    auto* page = static_cast<VehicleMeterPage*>(lv_timer_get_user_data(timer));
    page->Refresh();
}

void VehicleMeterPage::MeterAnimCallback(void* obj, int32_t value)
{
    auto* meter = static_cast<Meter*>(obj);
    lv_scale_set_line_needle_value(meter->scale, meter->line, meter->line_length, value);
    lv_label_set_text_fmt(meter->label, "%" PRId32, value);
}