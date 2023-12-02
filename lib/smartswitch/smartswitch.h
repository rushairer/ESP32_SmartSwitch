#ifndef __INCLUDE_SMARTSWITCH_H_
#define __INCLUDE_SMARTSWITCH_H_
#include <vector>
#include "sr501.h"

enum SmartSwitchMode
{
    NORMAL_MODE = 0,
    REACTIVE_MODE = 1
};

class SmartSwitch;

class SmartSwitch
{
public:
    SmartSwitch(int switchPin);
    virtual ~SmartSwitch();

    void setup();
    void turnOn();
    void turnOff();
    int isOn();
    void toggle();
    void setMode(SmartSwitchMode mode);
    SmartSwitchMode getMode();

protected:
    int _switchPin;
    SmartSwitchMode _mode;
};

#endif