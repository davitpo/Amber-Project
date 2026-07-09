#include <Arduino.h>
#include "Amber.hpp"

using amber::Amber;

Amber app;

void setup()
{
    Serial.begin(115200);
    delay(1000);

    app.begin();
}

void loop()
{
    app.update();
}