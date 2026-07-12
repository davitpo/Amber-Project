#include "SerialTransport.hpp"
#include <Arduino.h>
#include "Amber.hpp"

namespace amber {

void SerialTransport::update() {
    while (Serial.available() > 0) {
        char val = Serial.read();

        if (val == '\r' || val == '\n') {
            if (_overflow) {
                // Recover and reset overflow state cleanly upon reaching a newline
                Serial.println("ERROR=TOO_LONG");
                _length = 0;
                _overflow = false;
            } else if (_length > 0) {
                _buffer[_length] = '\0';
                
                // Submit command data to shared ACP routing path
                Amber::processCommand(CommandSource::UART, (const uint8_t*)_buffer, _length);
                _length = 0;
            }
            continue;
        }

        if (_overflow) {
            // Drop characters during overflow recovery
            continue;
        }

        if (_length >= 64) {
            _overflow = true;
            continue;
        }

        _buffer[_length++] = val;
    }
}

} // namespace amber
