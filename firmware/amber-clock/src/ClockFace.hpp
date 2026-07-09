#pragma once
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

namespace amber {

class ClockFace {
public:
    ClockFace() = default;
    void draw(LovyanGFX& tft);

private:
    void drawBackground(LovyanGFX& tft);
    void drawOuterRing(LovyanGFX& tft);
    void drawHourMarkers(LovyanGFX& tft);
    void drawCenter(LovyanGFX& tft);
};

} // namespace amber