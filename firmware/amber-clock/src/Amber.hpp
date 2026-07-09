#pragma once
#include "Display.hpp"
#include "Clock.hpp"
#include "ClockFace.hpp"
#include "ClockHands.hpp"

namespace amber {

class Amber {
private:
    Display _display;
    Clock _clock;
    ClockFace _clockFace;
    ClockHands _clockHands;

    void updateClock();
    void render();
    void renderBackground();
    void renderClock();
    void renderOverlay();
    void processEvents();

public:
    Amber() = default;
    void begin();
    void update();
};

} // namespace amber
