#pragma once
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

namespace amber {

class Display {
public:
    Display();
    void init();
    lgfx::LGFX_Device& getTft();

private:
    class LGFX_Custom : public lgfx::LGFX_Device {
        lgfx::Panel_GC9A01 _panel;
        lgfx::Bus_SPI _bus;
    public:
        LGFX_Custom();
    };

    LGFX_Custom _tft;
};

} // namespace amber