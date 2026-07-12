#pragma once
#include <stddef.h>
#include <stdint.h>

namespace amber {

class SerialTransport {
public:
    SerialTransport() = default;
    
    // Reads from Serial in a non-blocking manner and triggers execution upon line termination
    void update();

private:
    char _buffer[65];
    size_t _length = 0;
    bool _overflow = false;
};

} // namespace amber
