#pragma once
#include <stdint.h>

namespace amber {

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

    uint8_t hour() const { return _hour; }
    uint8_t minute() const { return _minute; }
    uint8_t second() const { return _second; }
};

} // namespace amber
