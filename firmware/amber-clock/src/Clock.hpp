#pragma once
#include <stdint.h>
#include "ClockAngles.hpp"
#include "Geometry.hpp"

namespace amber {

struct ClockTime {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

class Clock {
private:
    uint8_t _hour = 0;
    uint8_t _minute = 0;
    uint8_t _second = 0;

public:
    Clock() = default;
    
    void setTime(uint8_t h, uint8_t m, uint8_t s) {
        _hour = h;
        _minute = m;
        _second = s;
    }

    void tick() {
        _second++;
        if (_second >= 60) {
            _second = 0;
            _minute++;
            if (_minute >= 60) {
                _minute = 0;
                _hour++;
                if (_hour >= 24) {
                    _hour = 0;
                }
            }
        }
    }

    uint8_t hour() const { return _hour; }
    uint8_t minute() const { return _minute; }
    uint8_t second() const { return _second; }

    ClockTime getTime() const {
        return ClockTime{_hour, _minute, _second};
    }

    ClockAngles getAngles(float fractionalSecond = 0.0f) const {
        float secAndFrac = _second + fractionalSecond;
        float minAndFrac = _minute + secAndFrac / 60.0f;
        float hourAndFrac = (_hour % 12) + minAndFrac / 60.0f;

        return ClockAngles{
            (hourAndFrac * 30.0f - 90.0f) * Geometry::DEG_TO_RAD_LOCAL,
            (minAndFrac * 6.0f - 90.0f) * Geometry::DEG_TO_RAD_LOCAL,
            (secAndFrac * 6.0f - 90.0f) * Geometry::DEG_TO_RAD_LOCAL
        };
    }
};

} // namespace amber
