#pragma once
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "ClockAngles.hpp"

namespace amber {

class ClockHands {
public:
    ClockHands() = default;
    void draw(LovyanGFX& tft, const ClockAngles& angles);
};

} // namespace amber