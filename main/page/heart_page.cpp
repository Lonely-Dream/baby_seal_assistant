#include "heart_page.h"

#include <algorithm>
#include <cmath>

#include "esp_heap_caps.h"
#include "esp_log.h"

namespace {
    constexpr char kLogTag[] = "heart_page";
    constexpr uint32_t kBackgroundColor = 0x111416;
    constexpr uint8_t kBackgroundRed = 0x11;
    constexpr uint8_t kBackgroundGreen = 0x14;
    constexpr uint8_t kBackgroundBlue = 0x16;
    constexpr float kPi = 3.14159f;
    constexpr uint32_t kPulseFrameCount = 30;
    constexpr uint32_t kAnimationPeriodMs = 1000 / kPulseFrameCount;
    constexpr float kPulseAmplitude = 0.5f;
    constexpr float kMaximumPulseScale = 1.0f + kPulseAmplitude;
    constexpr float kVerticalMargin = 16.0f;
    constexpr float kMaxInsert = 1000.0f;

    constexpr uint16_t Rgb565(uint8_t red, uint8_t green, uint8_t blue)
    {
        return static_cast<uint16_t>(
            ((static_cast<uint16_t>(red) & 0xF8U) << 8U) |
            ((static_cast<uint16_t>(green) & 0xFCU) << 3U) |
            (static_cast<uint16_t>(blue) >> 3U)
            );
    }
}

HeartPage::HeartPage()
    : PageBase(PAGE_ID)
{ }

void HeartPage::OnCreate(lv_obj_t* parent)
{
    SetPageBackground(parent, kBackgroundColor);

    width_ = ScreenWidth();
    height_ = ScreenHeight();
    pixel_count_ = static_cast<size_t>(width_) * static_cast<size_t>(height_);
    pixels_ = static_cast<uint16_t*>(heap_caps_malloc(
        pixel_count_ * sizeof(*pixels_), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    if (pixels_ == nullptr) {
        pixels_ = static_cast<uint16_t*>(heap_caps_malloc(
            pixel_count_ * sizeof(*pixels_), MALLOC_CAP_8BIT));
    }
    if (pixels_ == nullptr) {
        ESP_LOGE(kLogTag, "Unable to allocate heart canvas buffer");
        return;
    }

    BuildLookupTables();
    canvas_ = lv_canvas_create(parent);
    lv_obj_set_size(canvas_, width_, height_);
    lv_obj_set_pos(canvas_, 0, 0);
    lv_canvas_set_buffer(canvas_, pixels_, width_, height_, LV_COLOR_FORMAT_RGB565);
    RenderFrame();
}

void HeartPage::OnEnter()
{
    if (pixels_ != nullptr && animation_timer_ == nullptr) {
        animation_timer_ = CreateTimer(AnimationTimer, kAnimationPeriodMs, this);
    }
}

void HeartPage::OnDestroy()
{
    canvas_ = nullptr;
    animation_timer_ = nullptr;
    if (pixels_ != nullptr) {
        heap_caps_free(pixels_);
        pixels_ = nullptr;
    }
    pixel_count_ = 0;
    width_ = 0;
    height_ = 0;
    center_y_ = 0.0f;
    frame_ = 0;
}

void HeartPage::BuildLookupTables()
{
    float min_y = 0.0f;
    float max_y = 0.0f;
    for (size_t index = 0; index < kHeartSampleCount; ++index) {
        const float t = (2.0f * kPi * static_cast<float>(index)) /
            static_cast<float>(kHeartSampleCount);
        const float sine = sinf(t);
        heart_points_[index] = Point{
            .x = 160.0f * sine * sine * sine,
            .y = 130.0f * cosf(t) - 50.0f * cosf(2.0f * t) -
                 20.0f * cosf(3.0f * t) - 10.0f * cosf(4.0f * t),
        };
        min_y = std::min(min_y, heart_points_[index].y);
        max_y = std::max(max_y, heart_points_[index].y);
    }

    const float min_center_y = kVerticalMargin + max_y * kMaximumPulseScale;
    const float max_center_y = static_cast<float>(height_ - 1) -
        kVerticalMargin + min_y * kMaximumPulseScale;
    const float ideal_center_y = (static_cast<float>(height_ - 1) +
        (min_y + max_y) * kMaximumPulseScale) / 2.0f;
    center_y_ = min_center_y <= max_center_y
        ? std::clamp(ideal_center_y, min_center_y, max_center_y)
        : static_cast<float>(height_) / 2.0f;

    const float max_log = 1.0f / logf(kMaxInsert);
    for (size_t index = 0; index < kHeatLutSize; ++index) {
        const float random = 1.0f +
            (static_cast<float>(index) * (kMaxInsert - 1.0f) /
                static_cast<float>(kHeatLutSize - 1));
        heat_lut_[index] = logf(random) * max_log;
    }
}

void HeartPage::RenderFrame()
{
    if (canvas_ == nullptr || pixels_ == nullptr) {
        return;
    }

    const uint16_t background = Rgb565(kBackgroundRed, kBackgroundGreen, kBackgroundBlue);
    std::fill_n(pixels_, pixel_count_, background);

    const float frame_rate = kPulseAmplitude * sinf(
        (2.0f * kPi * static_cast<float>(frame_)) /
        static_cast<float>(kPulseFrameCount));
    const uint16_t heart_color = Rgb565(
        255,
        static_cast<uint8_t>(184.0f + 56.0f * frame_rate),
        static_cast<uint8_t>(190.0f + 20.0f * frame_rate));
    const float center_x = static_cast<float>(width_) / 2.0f;

    for (const Point& point : heart_points_) {
        const float outside_x = point.x * (1.0f - frame_rate * NextHeat());
        const float outside_y = point.y * (1.0f - frame_rate * NextHeat());
        DrawPoint(
            static_cast<int32_t>(outside_x + center_x),
            static_cast<int32_t>(-outside_y + center_y_),
            heart_color);

        float inside_x = point.x * NextHeat();
        float inside_y = point.y * NextHeat();
        inside_x *= 1.0f - frame_rate * NextHeat();
        inside_y *= 1.0f - frame_rate * NextHeat();
        DrawPoint(
            static_cast<int32_t>(inside_x + center_x),
            static_cast<int32_t>(-inside_y + center_y_),
            heart_color);
    }

    lv_obj_invalidate(canvas_);
    frame_ = (frame_ + 1) % kPulseFrameCount;
}

float HeartPage::NextHeat()
{
    return heat_lut_[NextRandom() >> 24U];
}

uint32_t HeartPage::NextRandom()
{
    random_state_ ^= random_state_ << 13U;
    random_state_ ^= random_state_ >> 17U;
    random_state_ ^= random_state_ << 5U;
    return random_state_;
}

void HeartPage::DrawPoint(int32_t x, int32_t y, uint16_t color)
{
    if (x < 0 || x >= width_ || y < 0 || y >= height_) {
        return;
    }
    pixels_[static_cast<size_t>(y) * static_cast<size_t>(width_) + static_cast<size_t>(x)] = color;
}

void HeartPage::AnimationTimer(lv_timer_t* timer)
{
    auto* page = static_cast<HeartPage*>(lv_timer_get_user_data(timer));
    page->RenderFrame();
}