#pragma once
#include <stdint.h>

namespace amber {

class HealthMonitor {
private:
    bool _enabled = false;
    uint32_t _intervalMs = 1000;
    uint32_t _lastRunMs = 0;

public:
    HealthMonitor() = default;

    bool isEnabled() const { return _enabled; }
    void setEnabled(bool enable) { _enabled = enable; }

    uint32_t getInterval() const { return _intervalMs; }
    void setInterval(uint32_t ms) { _intervalMs = ms; }

    bool shouldRun(uint32_t currentMs) {
        if (!_enabled) return false;
        if (currentMs - _lastRunMs >= _intervalMs) {
            _lastRunMs = currentMs;
            return true;
        }
        return false;
    }
};

} // namespace amber
