#include "Geometry.hpp"
#include "Theme.hpp"

extern "C" float cosf(float);
extern "C" float sinf(float);

namespace amber {

int16_t Geometry::polarToX(float angleRad, float radius) {
    return Theme::Clock::CenterX + cosf(angleRad) * radius;
}

int16_t Geometry::polarToY(float angleRad, float radius) {
    return Theme::Clock::CenterY + sinf(angleRad) * radius;
}

float Geometry::timeToAngleSecMin(uint8_t value) {
    return (value * 6.0f - 90.0f) * DEG_TO_RAD_LOCAL;
}

float Geometry::timeToAngleHour(uint8_t hour, uint8_t min) {
    return ((hour % 12) * 30.0f + min * 0.5f - 90.0f) * DEG_TO_RAD_LOCAL;
}

Point Geometry::polar(float angleRad, float radius) {
    return Point{polarToX(angleRad, radius), polarToY(angleRad, radius)};
}

Point Geometry::hourMarkerInner(uint8_t index) {
    float angle = (index * 30.0f - 90.0f) * DEG_TO_RAD_LOCAL;
    return polar(angle, Theme::Clock::RTickInnerMajor);
}

Point Geometry::hourMarkerOuter(uint8_t index) {
    float angle = (index * 30.0f - 90.0f) * DEG_TO_RAD_LOCAL;
    return polar(angle, Theme::Clock::RTickOuter);
}

} // namespace amber