#include "Display.hpp"
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

Display::Display() : _tft() {}

void Display::init() {
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    _tft.init();
}

lgfx::LGFX_Device& Display::getTft() {
    return _tft;
}

} // namespace amber