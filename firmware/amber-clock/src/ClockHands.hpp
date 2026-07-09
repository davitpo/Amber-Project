#pragma once
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "Clock.hpp"

namespace amber {

class ClockHands {
public:
    ClockHands() = default;
    void draw(lgfx::LGFX_Device& tft, const Clock& clock);
};

} // namespace amber