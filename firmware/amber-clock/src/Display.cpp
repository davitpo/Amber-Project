#include "Display.hpp"
#include "Logger.hpp"
#include <Arduino.h>

#define TFT_BL   3
#define TFT_RST  -1
#define TFT_DC   2
#define TFT_CS   10
#define TFT_SCLK 6
#define TFT_MOSI 7

// Constexpr 8-bit Q-Format gamma 2.2 approximation mapping table for user percent input (0..100)
static const uint8_t PROGMEM GammaTable[101] = {
    0,   0,   0,   0,   0,   1,   1,   1,   1,   2,
    2,   2,   3,   3,   4,   4,   5,   5,   6,   7,
    7,   8,   9,   10,  11,  12,  13,  14,  15,  16,
    17,  18,  19,  21,  22,  23,  25,  26,  28,  29,
    31,  33,  34,  36,  38,  40,  42,  44,  46,  48,
    50,  52,  54,  57,  59,  61,  64,  66,  69,  71,
    74,  77,  79,  82,  85,  88,  91,  94,  97,  100,
    103, 106, 110, 113, 116, 120, 123, 127, 130, 134,
    138, 142, 145, 149, 153, 157, 161, 165, 170, 174,
    178, 183, 187, 192, 196, 201, 206, 210, 215, 220,
    255 // Ensures full max capacity on 100 percent
};

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
    digitalWrite(TFT_BL, LOW); // Initialize low first to avoid bright flashes during boot
    
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

    // Apply startup default brightness
    setBrightnessPercent(_brightnessPercent);

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

uint8_t Display::getBrightnessPercent() const {
    return _brightnessPercent;
}

void Display::setBrightnessPercent(uint8_t percent) {
    if (percent > 100) {
        percent = 100;
    }
    _brightnessPercent = percent;

    // Retrieve perceptual gamma mapped PWM level from look-up table
    uint8_t pwmVal = pgm_read_byte(&GammaTable[percent]);
    
    // Output active-high PWM signaling directly to GPIO3 using stable Arduino framework analogWrite API
    analogWrite(TFT_BL, pwmVal);

    char logMsg[64];
    snprintf(logMsg, sizeof(logMsg), "DISPLAY BRIGHTNESS=%d PWM=%d", percent, pwmVal);
    LOG_INFO(logMsg);
}

lgfx::LGFX_Device& Display::getTft() {
    return _tft;
}

} // namespace amber