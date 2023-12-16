#ifndef __INCLUDE_SR501_H_
#define __INCLUDE_SR501_H_
#include <Arduino.h>

class SR501Service;

class SR501Service
{
public:
    SR501Service(int pin = 1);
    virtual ~SR501Service();

    void setup();
    void loop();
    int getStatus();

protected:
    int _status;
    int _pin;
};

#endif