#pragma once
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "RenderMode.hpp"

namespace amber {

class Display {
public:
    Display();
    bool begin();
    LovyanGFX& target();
    void pushFrame();
    RenderMode renderMode() const;
    uint32_t freeHeap() const;
    uint32_t minFreeHeap() const;
    lgfx::LGFX_Device& getTft();
    
    // Read/Write display backlight brightness (0..100) percent
    uint8_t getBrightnessPercent() const;
    void setBrightnessPercent(uint8_t percent);

private:
    class LGFX_Custom : public lgfx::LGFX_Device {
        lgfx::Panel_GC9A01 _panel;
        lgfx::Bus_SPI _bus;
    public:
        LGFX_Custom();
    };

    LGFX_Custom _tft;
    LGFX_Sprite _sprite;
    RenderMode _mode = RenderMode::DirectSafe;
    bool _spriteAllocated = false;
    uint8_t _brightnessPercent = 100; // default initial state value
};

} // namespace amber