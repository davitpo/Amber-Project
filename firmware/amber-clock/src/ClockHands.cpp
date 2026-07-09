#include "ClockHands.hpp"
#include "Theme.hpp"
#include "Geometry.hpp"

namespace amber {

void ClockHands::draw(LovyanGFX& tft, const ClockAngles& angles) {
    // Hour hand setup using prepared angles
    int16_t hx = Geometry::polarToX(angles.hourAngleRad, Theme::Clock::RTickInnerMajor - 25);
    int16_t hy = Geometry::polarToY(angles.hourAngleRad, Theme::Clock::RTickInnerMajor - 25);
    tft.drawLine(Theme::Clock::CenterX, Theme::Clock::CenterY, hx, hy, Theme::Colors::Amber);

    // Minute hand setup using prepared angles
    int16_t mx = Geometry::polarToX(angles.minuteAngleRad, Theme::Clock::RTickInnerMajor - 10);
    int16_t my = Geometry::polarToY(angles.minuteAngleRad, Theme::Clock::RTickInnerMajor - 10);
    tft.drawLine(Theme::Clock::CenterX, Theme::Clock::CenterY, mx, my, Theme::Colors::Amber);

    // Second hand setup using prepared angles
    int16_t sx = Geometry::polarToX(angles.secondAngleRad, Theme::Clock::RTickInnerMajor - 5);
    int16_t sy = Geometry::polarToY(angles.secondAngleRad, Theme::Clock::RTickInnerMajor - 5);
    tft.drawLine(Theme::Clock::CenterX, Theme::Clock::CenterY, sx, sy, Theme::Colors::Amber);
}

} // namespace amber