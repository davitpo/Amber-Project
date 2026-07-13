#define LGFX_USE_V1
#include <Arduino.h>
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_GC9A01 _panel;
  lgfx::Bus_SPI _bus;

public:
  LGFX()
  {
    auto bus_cfg = _bus.config();
    bus_cfg.spi_host = SPI2_HOST;
    bus_cfg.spi_mode = 0;
    bus_cfg.freq_write = 40000000;
    bus_cfg.freq_read = 20000000;
    bus_cfg.spi_3wire = true;
    bus_cfg.use_lock = true;
    bus_cfg.dma_channel = SPI_DMA_CH_AUTO;
    bus_cfg.pin_sclk = 6;
    bus_cfg.pin_mosi = 7;
    bus_cfg.pin_miso = -1;
    bus_cfg.pin_dc = 2;
    _bus.config(bus_cfg);
    _panel.setBus(&_bus);

    auto panel_cfg = _panel.config();
    panel_cfg.pin_cs = 10;
    panel_cfg.pin_rst = -1;
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
};

LGFX tft;

#define AMBER 0xFD20

constexpr uint8_t TFT_BL = 3;

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("Amber Project");
    Serial.println("Backlight PWM test");

    tft.init();
    tft.fillScreen(TFT_BLACK);

    // Draw something clearly visible while brightness changes.
    tft.fillCircle(120, 120, 60, AMBER);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(middle_center);
    tft.drawString("PWM TEST", 120, 120);

    pinMode(TFT_BL, OUTPUT);

    Serial.println("Starting fade...");
}


void loop()
{
    Serial.println("Brightness: 0 -> 255");

    for (int value = 0; value <= 255; ++value)
    {
        analogWrite(TFT_BL, value);
        delay(12);
    }

    delay(500);

    Serial.println("Brightness: 255 -> 0");

    for (int value = 255; value >= 0; --value)
    {
        analogWrite(TFT_BL, value);
        delay(12);
    }

    delay(500);
}