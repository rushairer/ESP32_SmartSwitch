#include "smartswitch.h"
#include <Arduino.h>

SmartSwitch::SmartSwitch(int switchPin)
{
    _switchPin = switchPin;
    setup();
    setMode(NORMAL_MODE);
}

SmartSwitch::~SmartSwitch()
{
}

void SmartSwitch::setup()
{
    pinMode(_switchPin, OUTPUT);
    digitalWrite(_switchPin, HIGH);
}

void SmartSwitch::turnOn()
{
    digitalWrite(_switchPin, HIGH);
}

void SmartSwitch::turnOff()
{
    digitalWrite(_switchPin, LOW);
}

int SmartSwitch::isOn()
{
    return digitalRead(_switchPin);
}

void SmartSwitch::toggle()
{
    digitalWrite(_switchPin, !digitalRead(_switchPin));
}

void SmartSwitch::setMode(SmartSwitchMode mode)
{
    _mode = mode;
}

SmartSwitchMode SmartSwitch::getMode()
{
    return _mode;
}