#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "lvgl_nav_kit/page_base.h"

class HeartPage final : public ui::PageBase {
public:
    static constexpr const char* PAGE_ID = "HeartPage";

    HeartPage();

    void OnCreate(lv_obj_t* parent) override;
    void OnEnter() override;
    void OnDestroy() override;

private:
    struct Point {
        float x;
        float y;
    };

    static constexpr size_t kHeartSampleCount = 4096;
    static constexpr size_t kHeatLutSize = 256;

    void BuildLookupTables();
    void RenderFrame();
    float NextHeat();
    uint32_t NextRandom();
    void DrawPoint(int32_t x, int32_t y, uint16_t color);
    static void AnimationTimer(lv_timer_t* timer);

    std::array<Point, kHeartSampleCount> heart_points_{ };
    std::array<float, kHeatLutSize> heat_lut_{ };
    lv_obj_t* canvas_ = nullptr;
    lv_timer_t* animation_timer_ = nullptr;
    uint16_t* pixels_ = nullptr;
    size_t pixel_count_ = 0;
    int32_t width_ = 0;
    int32_t height_ = 0;
    float center_y_ = 0.0f;
    uint32_t frame_ = 0;
    uint32_t random_state_ = 0x2E1A4D73;
};