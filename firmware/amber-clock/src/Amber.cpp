#include "Amber.hpp"
#include "Logger.hpp"
#include <Arduino.h>

namespace amber {

void Amber::begin() {
    _clock.setTime(0, 36, 0);
    _lastTickMs = millis();
    _lastRenderMs = millis();
    _lastHealthLogMs = millis();

    LOG_INFO("Clock initialized");

    _display.begin();
    
    // Initial display print
    renderFrame(0.0f);

    LOG_INFO("Amber initialized");
}

void Amber::update() {
    updateClock();
    processEvents();
    logHealth();
}

void Amber::updateClock() {
    uint32_t currentMs = millis();
    
    // Cascading time progression tick
    if (currentMs - _lastTickMs >= TickIntervalMs) {
        _lastTickMs += TickIntervalMs;
        _clock.tick();
    }

    // Adapt frame render rate according to selected platform buffer capacity
    uint32_t interval = (_display.renderMode() == RenderMode::FullSprite) ? RenderIntervalMsFull : RenderIntervalMsDirect;
    if (currentMs - _lastRenderMs >= interval) {
        _lastRenderMs = currentMs;
        
        float fractionalSecond = 0.0f;
        if (_display.renderMode() == RenderMode::FullSprite) {
            uint32_t elapsedMs = currentMs - _lastTickMs;
            if (elapsedMs > TickIntervalMs) {
                elapsedMs = TickIntervalMs;
            }
            fractionalSecond = static_cast<float>(elapsedMs) / 1000.0f;
            if (fractionalSecond > 0.999f) {
                fractionalSecond = 0.999f;
            }
        }
        
        renderFrame(fractionalSecond);
        _renderedFramesThisSecond++;
    }
}

void Amber::renderFrame(float fractionalSecond) {
    auto& canvas = _display.target();
    renderStaticLayer(canvas);
    renderDynamicLayer(canvas, fractionalSecond);
    _display.pushFrame();
}

void Amber::renderStaticLayer(LovyanGFX& target) {
    _clockFace.draw(target);
}

void Amber::renderDynamicLayer(LovyanGFX& target, float fractionalSecond) {
    _clockHands.draw(target, _clock.getAngles(fractionalSecond));
}

void Amber::logHealth() {
    uint32_t currentMs = millis();
    if (currentMs - _lastHealthLogMs >= HealthLogIntervalMs) {
        _lastHealthLogMs += HealthLogIntervalMs;
        _fps = _renderedFramesThisSecond;
        _renderedFramesThisSecond = 0;

        char buf[80];
        snprintf(buf, sizeof(buf), "T=%02d:%02d:%02d | mode=%s | heap=%u | min=%u | fps=%u",
                 _clock.hour(), _clock.minute(), _clock.second(),
                 (_display.renderMode() == RenderMode::FullSprite) ? "FullSprite" : "DirectSafe",
                 _display.freeHeap(), _display.minFreeHeap(), _fps);
        LOG_INFO(buf);
    }
}

void Amber::processEvents() {
    // Left available for button clicks / future integrations
}

} // namespace amber
