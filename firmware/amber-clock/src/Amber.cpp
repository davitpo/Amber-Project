#include "Amber.hpp"
#include "Logger.hpp"
#include <Arduino.h>

namespace amber {

void Amber::begin() {
    //00:36 E36 time started
    _clock.setTime(0, 36, 0); 

    LOG_INFO("Clock initialized");

    _display.init();
    
    // Initial paint rendering flow
    render();

    LOG_INFO("Amber Circle done");
}

void Amber::update() {
    updateClock();
    render();
    processEvents();
}

void Amber::updateClock() {
    // Pipeline stage placeholder (no animations yet)
}

void Amber::render() {
    renderBackground();
    renderClock();
    renderOverlay();
}

void Amber::renderBackground() {
    // Let ClockFace clear background on display
    _clockFace.draw(_display.getTft());
}

void Amber::renderClock() {
    // Paint dynamic layer
    _clockHands.draw(_display.getTft(), _clock);
}

void Amber::renderOverlay() {
    // Dynamic overlay placeholder
}

void Amber::processEvents() {
    LOG_INFO("Alive");
}

} // namespace amber
