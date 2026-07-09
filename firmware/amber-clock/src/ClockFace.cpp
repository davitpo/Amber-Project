#include "ClockFace.hpp"
#include "Theme.hpp"
#include "Geometry.hpp"

namespace amber {

void ClockFace::draw(lgfx::LGFX_Device& tft) {
    drawBackground(tft);
    drawOuterRing(tft);
    drawHourMarkers(tft);
    drawCenter(tft);
}

void ClockFace::drawBackground(lgfx::LGFX_Device& tft) {
    tft.fillScreen(Theme::Colors::Black);
}

void ClockFace::drawOuterRing(lgfx::LGFX_Device& tft) {
    tft.drawCircle(Theme::Clock::CenterX, Theme::Clock::CenterY, Theme::Clock::ROuter, Theme::Colors::Amber);
    tft.drawCircle(Theme::Clock::CenterX, Theme::Clock::CenterY, Theme::Clock::ROuter - 1, Theme::Colors::Amber);
}

void ClockFace::drawHourMarkers(lgfx::LGFX_Device& tft) {
    for (uint8_t i = 0; i < 12; i++) {
        Point p1 = Geometry::hourMarkerInner(i);
        Point p2 = Geometry::hourMarkerOuter(i);
        tft.drawLine(p1.x, p1.y, p2.x, p2.y, Theme::Colors::Amber);
    }
}

void ClockFace::drawCenter(lgfx::LGFX_Device& tft) {
    tft.fillCircle(Theme::Clock::CenterX, Theme::Clock::CenterY, 3, Theme::Colors::Amber);
}

} // namespace amber