#pragma once
#include "Display.hpp"
#include "Clock.hpp"
#include "ClockFace.hpp"
#include "ClockHands.hpp"
#include "BleService.hpp"
#include "SerialTransport.hpp"
#include "HealthMonitor.hpp"
#include "CommandSource.hpp"

namespace amber {

class Amber {
private:
    Display _display;
    Clock _clock;
    ClockFace _clockFace;
    ClockHands _clockHands;
    BleService _ble;
    SerialTransport _serialTransport;
    HealthMonitor _healthMonitor;
    
    // FPS tracking variables decoupled from output loops
    uint32_t _lastTickMs = 0;
    uint32_t _lastRenderMs = 0;
    uint32_t _lastFpsMetricsMs = 0;
    uint32_t _frameCount = 0;
    uint32_t _fps = 0;

    static constexpr uint32_t TickIntervalMs = 1000;
    static constexpr uint32_t RenderIntervalMsFull = 33;    // Smooth FPS
    static constexpr uint32_t RenderIntervalMsDirect = 1000; // Discrete direct

    void updateClock();
    void calculateFps();
    void renderStaticLayer(LovyanGFX& target);
    void renderDynamicLayer(LovyanGFX& target, float fractionalSecond);
    void renderFrame(float fractionalSecond);
    void processEvents();
    void runPeriodicTelemetry();

public:
    Amber() = default;
    void begin();
    void update();

    uint8_t getClockHour() const { return _clock.hour(); }
    uint8_t getClockMinute() const { return _clock.minute(); }
    uint8_t getClockSecond() const { return _clock.second(); }

    uint8_t getBrightnessPercent() const { return _display.getBrightnessPercent(); }
    void setBrightnessPercent(uint8_t percent) { _display.setBrightnessPercent(percent); }

    void setLocalTime(uint8_t h, uint8_t m, uint8_t s) {
        _clock.setTime(h, m, s);
        _lastTickMs = millis();
    }
    
    // Read local time layout as standard ClockTime structure
    ClockTime getLocalTime() const { return _clock.getTime(); }

    const char* getRenderModeStr() const { return (_display.renderMode() == RenderMode::FullSprite) ? "FullSprite" : "DirectSafe"; }
    bool isBleConnected() const { return _ble.isConnected(); }
    uint32_t getFreeHeap() const { return _display.freeHeap(); }
    uint32_t getMinFreeHeap() const { return _display.minFreeHeap(); }
    uint32_t getFps() const { return _fps; }

    static void processCommand(CommandSource source, const uint8_t* data, size_t length);
};

// Global reference wrapper accessor for Acp command response builders
extern Amber* GlobalAmberInstance;

} // namespace amber
