#pragma once
#include <stdint.h>

namespace amber {

struct Theme {
    struct Colors {
        static constexpr uint16_t Amber = 0xFBE0; //(255, 126, 0)
        static constexpr uint16_t Black = 0x0000;
    };

    struct Clock {
        static constexpr int16_t CenterX = 120;
        static constexpr int16_t CenterY = 120;
        static constexpr int16_t ROuter = 112;
        static constexpr int16_t RTickOuter = 104;
        static constexpr int16_t RTickInnerMajor = 92;
        static constexpr int16_t RTickInnerMinor = 98;
    };

    struct Hands {
        // Retained for future configuration options
    };
};

} // namespace amber