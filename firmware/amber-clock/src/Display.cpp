#include "Display.hpp"
#include "Logger.hpp"
#include <Arduino.h>

#define TFT_BL   3
#define TFT_RST  -1
#define TFT_DC   2
#define TFT_CS   10
#define TFT_SCLK 6
#define TFT_MOSI 7

namespace amber {

Display::LGFX_Custom::LGFX_Custom() {
    auto bus_cfg = _bus.config();
    bus_cfg.spi_host = SPI2_HOST;
    bus_cfg.spi_mode = 0;
    bus_cfg.freq_write = 40000000;
    bus_cfg.freq_read = 20000000;
    bus_cfg.spi_3wire = true;
    bus_cfg.use_lock = true;
    bus_cfg.dma_channel = SPI_DMA_CH_AUTO;
    bus_cfg.pin_sclk = TFT_SCLK;
    bus_cfg.pin_mosi = TFT_MOSI;
    bus_cfg.pin_miso = -1;
    bus_cfg.pin_dc = TFT_DC;
    _bus.config(bus_cfg);
    _panel.setBus(&_bus);

    auto panel_cfg = _panel.config();
    panel_cfg.pin_cs = TFT_CS;
    panel_cfg.pin_rst = TFT_RST;
    panel_cfg.pin_busy = -1;
    panel_cfg.memory_width = 240;
    panel_cfg.memory_height = 240;
    panel_cfg.panel_width = 240;
    panel_cfg.panel_height = 240;
    panel_cfg.offset_x = 0;
    panel_cfg.offset_y = 0;
    panel_cfg.invert = true;
    panel_cfg.rgb_order = false;
    panel_cfg.bus_shared = false;
    _panel.config(panel_cfg);

    setPanel(&_panel);
}

Display::Display() : _tft(), _sprite(&_tft) {}

bool Display::begin() {
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    
    _tft.init();

    _sprite.setPsram(false);
    _sprite.setColorDepth(16);
    _sprite.setBuffer(nullptr, 240, 240, 16);
    void* ptr = _sprite.createSprite(240, 240);
    
    if (ptr != nullptr) {
        _spriteAllocated = true;
        _mode = RenderMode::FullSprite;
        LOG_INFO("Display initialization: LGFX Sprite allocated successfully (FullSprite mode).");
    } else {
        _spriteAllocated = false;
        _mode = RenderMode::DirectSafe;
        LOG_INFO("Display initialization: LGFX Sprite allocation failed! Falling back (DirectSafe mode).");
    }

    return true;
}

LovyanGFX& Display::target() {
    if (_mode == RenderMode::FullSprite && _spriteAllocated) {
        return _sprite;
    }
    return _tft;
}

void Display::pushFrame() {
    if (_mode == RenderMode::FullSprite && _spriteAllocated) {
        _sprite.pushSprite(0, 0);
    }
}

RenderMode Display::renderMode() const {
    return _mode;
}

uint32_t Display::freeHeap() const {
    return ESP.getFreeHeap();
}

uint32_t Display::minFreeHeap() const {
    return ESP.getMinFreeHeap();
}

lgfx::LGFX_Device& Display::getTft() {
    return _tft;
}

} // namespace amber