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
    
    uint32_t _lastTickMs = 0;
    uint32_t _lastRenderMs = 0;
    uint32_t _lastHealthLogMs = 0;
    uint32_t _renderedFramesThisSecond = 0;
    uint32_t _fps = 0;

    static constexpr uint32_t TickIntervalMs = 1000;
    static constexpr uint32_t RenderIntervalMsFull = 33;    // Smooth FPS
    static constexpr uint32_t RenderIntervalMsDirect = 1000; // Discrete direct
    static constexpr uint32_t HealthLogIntervalMs = 1000;

    void updateClock();
    void renderStaticLayer(LovyanGFX& target);
    void renderDynamicLayer(LovyanGFX& target, float fractionalSecond);
    void renderFrame(float fractionalSecond);
    void processEvents();
    void logHealth();

public:
    Amber() = default;
    void begin();
    void update();
};

} // namespace amber
