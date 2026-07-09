#include "ClockHands.hpp"
#include "Theme.hpp"
#include "Geometry.hpp"

namespace amber {

void ClockHands::draw(lgfx::LGFX_Device& tft, const Clock& clock) {
    // Hour hand setup
    float hourAngle = Geometry::timeToAngleHour(clock.hour(), clock.minute());
    int16_t hx = Geometry::polarToX(hourAngle, Theme::Clock::RTickInnerMajor - 25);
    int16_t hy = Geometry::polarToY(hourAngle, Theme::Clock::RTickInnerMajor - 25);
    tft.drawLine(Theme::Clock::CenterX, Theme::Clock::CenterY, hx, hy, Theme::Colors::Amber);

    // Minute hand setup
    float minAngle = Geometry::timeToAngleSecMin(clock.minute());
    int16_t mx = Geometry::polarToX(minAngle, Theme::Clock::RTickInnerMajor - 10);
    int16_t my = Geometry::polarToY(minAngle, Theme::Clock::RTickInnerMajor - 10);
    tft.drawLine(Theme::Clock::CenterX, Theme::Clock::CenterY, mx, my, Theme::Colors::Amber);
}

} // namespace amber