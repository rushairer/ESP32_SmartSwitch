#ifndef __INCLUDE_SMARTSWITCH_MANAGER_H_
#define __INCLUDE_SMARTSWITCH_MANAGER_H_
#include <WebServer.h>
#include "sr501.h"
#include "smartswitch.h"

class SmartSwitchManager;

class SmartSwitchManager
{
public:
    SmartSwitchManager(
        std::vector<SmartSwitch *> switches,
        int serverPort,
        SR501Service *sr501);
    virtual ~SmartSwitchManager();

    void setupSwitchs();
    void turnOnSwitch(int index);
    void turnOffSwitch(int index);
    void toggleSwitch(int index);
    int getToggleStatus(int index);
    void turnOnAllSwitch();
    void turnOffAllSwitch();
    void toggleAllSwitch();
    void setMode(int index, SmartSwitchMode mode);
    SmartSwitchMode getMode(int index);
    void setAllMode(SmartSwitchMode mode);

    void setup();
    void loop();

protected:
    SR501Service *_sr501;
    WebServer _webServer;
    std::vector<SmartSwitch *> _switches;

private:
    void handleIndex();
    void handleGetToggle();
    void handleChangeToggle();
    void handleGeteMode();
    void handleChangeMode();
};

#endif