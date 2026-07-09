#pragma once
#include <stdint.h>

namespace amber {

struct Point {
    int16_t x;
    int16_t y;
};

class Geometry {
public:
    static constexpr float DEG_TO_RAD_LOCAL = 3.14159265358979323846f / 180.0f;

    static int16_t polarToX(float angleRad, float radius);
    static int16_t polarToY(float angleRad, float radius);
    static float timeToAngleSecMin(uint8_t value);
    static float timeToAngleHour(uint8_t hour, uint8_t min);

    static Point polar(float angleRad, float radius);
    static Point hourMarkerInner(uint8_t index);
    static Point hourMarkerOuter(uint8_t index);
};

} // namespace amber