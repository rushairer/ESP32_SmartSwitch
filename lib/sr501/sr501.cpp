#include "sr501.h"

SR501Service::SR501Service(int pin, uint8_t bits)
{
    _status = 0;
    _pin = pin;
    analogReadResolution(bits);
}

SR501Service::~SR501Service()
{
}

int SR501Service::getStatus()
{
    return _status;
}

void SR501Service::loop()
{
    _status = analogRead(_pin);
    delay(10);
}