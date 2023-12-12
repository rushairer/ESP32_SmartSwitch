#include "sr501.h"

SR501Service::SR501Service(int pin)
{
    _status = 0;
    _pin = pin;
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
    if (analogRead(_pin) > 4000)
    {
        _status = 1;
    }
    else
    {
        _status = 0;
    }

    delay(10);
}