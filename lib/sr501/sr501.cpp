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

void SR501Service::setup()
{
    adcAttachPin(_pin);
}

void SR501Service::loop()
{
    int adcValue = analogRead(_pin);
    if (adcValue > 4000)
    {
        _status = 1;
    }
    else
    {
        _status = 0;
    }

    Serial.printf("ADC Value(Pin %d):%d, SR501 status: %d \r\n", _pin, adcValue, _status);

    delay(10);
}