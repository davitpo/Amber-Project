#pragma once
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

namespace amber {

class ClockFace {
public:
    ClockFace() = default;
    void draw(lgfx::LGFX_Device& tft);

private:
    void drawBackground(lgfx::LGFX_Device& tft);
    void drawOuterRing(lgfx::LGFX_Device& tft);
    void drawHourMarkers(lgfx::LGFX_Device& tft);
    void drawCenter(lgfx::LGFX_Device& tft);
};

} // namespace amber