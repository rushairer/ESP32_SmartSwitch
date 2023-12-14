#include "smartswitchmanager.h"
#include <ArduinoJson.h>
#include <TimeLib.h>

SmartSwitchManager::SmartSwitchManager(std::vector<SmartSwitch *> switches, int serverPort, SR501Service *sr501)
    : _webServer(serverPort),
      _switches(switches),
      _sr501(sr501)
{
}

SmartSwitchManager::~SmartSwitchManager()
{
    _webServer.stop();
}

void SmartSwitchManager::setup()
{
    setupSwitchs();

    std::function<void(void)> handleIndex = std::bind(&SmartSwitchManager::handleIndex, this);
    std::function<void(void)> handleGetToggle = std::bind(&SmartSwitchManager::handleGetToggle, this);
    std::function<void(void)> handleGeteMode = std::bind(&SmartSwitchManager::handleGeteMode, this);
    std::function<void(void)> handleChangeToggle = std::bind(&SmartSwitchManager::handleChangeToggle, this);
    std::function<void(void)> handleChangeMode = std::bind(&SmartSwitchManager::handleChangeMode, this);

    _webServer.on(
        "/",
        HTTP_GET,
        handleIndex);

    _webServer.on(
        "/toggle",
        HTTP_GET,
        handleGetToggle);

    _webServer.on(
        "/toggle",
        HTTP_POST,
        handleChangeToggle);

    _webServer.on(
        "/mode",
        HTTP_GET,
        handleGeteMode);

    _webServer.on(
        "/mode",
        HTTP_POST,
        handleChangeMode);

    _webServer.begin();
}

void SmartSwitchManager::setupSwitchs()
{
    for (SmartSwitch *smartswitch : _switches)
    {
        smartswitch->setup();
    }
}

void SmartSwitchManager::turnOnSwitch(int index)
{
    if (index < _switches.size())
    {
        _switches[index]->turnOn();
    }
}

void SmartSwitchManager::turnOffSwitch(int index)
{
    if (index < _switches.size())
    {
        _switches[index]->turnOff();
    }
}

void SmartSwitchManager::toggleSwitch(int index)
{
    if (index < _switches.size())
    {
        _switches[index]->toggle();
    }
}

int SmartSwitchManager::getToggleStatus(int index)
{
    if (index < _switches.size())
    {
        return _switches[index]->isOn();
    }

    return 0;
}

void SmartSwitchManager::turnOnAllSwitch()
{
    for (SmartSwitch *smartswitch : _switches)
    {
        smartswitch->turnOn();
    }
}
void SmartSwitchManager::turnOffAllSwitch()
{
    for (SmartSwitch *smartswitch : _switches)
    {
        smartswitch->turnOff();
    }
}
void SmartSwitchManager::toggleAllSwitch()
{
    for (SmartSwitch *smartswitch : _switches)
    {
        smartswitch->toggle();
    }
}

void SmartSwitchManager::setMode(int index, SmartSwitchMode mode)
{
    if (index < _switches.size())
    {
        _switches[index]->setMode(mode);
    }
}

SmartSwitchMode SmartSwitchManager::getMode(int index)
{
    if (index < _switches.size())
    {
        return _switches[index]->getMode();
    }

    return NORMAL_MODE;
}

void SmartSwitchManager::setAllMode(SmartSwitchMode mode)
{
    for (SmartSwitch *smartswitch : _switches)
    {
        smartswitch->setMode(mode);
    }
}

void SmartSwitchManager::handleIndex()
{
    String indexHTMLString = R"***(
<!DOCTYPE html>
<html>
    <head>
        <title>SmartSwitch</title>
        <meta
            name="viewport"
            content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=0"
        />
        <style>
            body {
                background: #333;
                font-family: sans-serif;
                font-size: 14px;
                color: #777;
            }

            .switchBox {
                padding: 10px;
                display: flex;
                justify-items: center;
                align-items: center;
            }

            .switch {
                position: relative;
                display: inline-block;
                width: 60px;
                height: 34px;
                margin: 0 10px;
            }

            .switch input {
                opacity: 0;
                width: 0;
                height: 0;
            }

            .slider {
                position: absolute;
                cursor: pointer;
                top: 0;
                left: 0;
                right: 0;
                bottom: 0;
                background-color: #ccc;
                -webkit-transition: 0.4s;
                transition: 0.4s;
            }

            .slider:before {
                position: absolute;
                content: '';
                height: 26px;
                width: 26px;
                left: 4px;
                bottom: 4px;
                background-color: white;
                -webkit-transition: 0.4s;
                transition: 0.4s;
            }

            input:checked + .slider {
                background-color: #2196f3;
            }

            input:focus + .slider {
                box-shadow: 0 0 1px #2196f3;
            }

            input:checked + .slider:before {
                -webkit-transform: translateX(26px);
                -ms-transform: translateX(26px);
                transform: translateX(26px);
            }

            .slider.round {
                border-radius: 34px;
            }

            .slider.round:before {
                border-radius: 50%;
            }
        </style>
    </head>
    <body>
        <h3>Smart Switch</h3>
        <div class="switchBox">
            <label class="switch">
                <input type="checkbox" id="switch0" onchange="toggle(0)" />
                <span class="slider round"></span>
            </label>
            <label for="switch0">Switch 1</label>
        </div>
        <div class="switchBox">
            <label class="switch">
                <input type="checkbox" id="switch1" onchange="toggle(1)" />
                <span class="slider round"></span>
            </label>
            <label for="switch1">Switch 2</label>
        </div>
        <div class="switchBox">
            <label class="switch">
                <input type="checkbox" id="switchauto0" onchange="mode(0)" />
                <span class="slider round"></span>
            </label>
            <label for="switchauto0">Switch 1 Auto</label>
        </div>
        <div class="switchBox">
            <label class="switch">
                <input type="checkbox" id="switchauto1" onchange="mode(1)" />
                <span class="slider round"></span>
            </label>
            <label for="switchauto1">Switch 2 Auto</label>
        </div>
        <script>
            function toggle(index) {
                var isChecked = document.getElementById(
                    'switch' + index
                ).checked

                console.log(index, isChecked)

                var xhr = new XMLHttpRequest()
                var data = JSON.stringify({
                    index,
                    action: isChecked ? 'on' : 'off',
                })
                xhr.onreadystatechange = function () {
                    if (xhr.readyState === 4 && xhr.status === 200) {
                        refresh()
                    }
                    if (xhr.readyState === 4 && xhr.status !== 200) {
                        console.error('Failed!')
                    }
                }
                xhr.open('POST', '/toggle', true)
                xhr.setRequestHeader('Content-Type', 'application/json')
                xhr.send(data)
            }

            function mode(index) {
                var isChecked = document.getElementById(
                    'switchauto' + index
                ).checked

                console.log(index, isChecked)

                var xhr = new XMLHttpRequest()
                var data = JSON.stringify({
                    index,
                    mode: isChecked ? 1 : 0,
                })
                xhr.onreadystatechange = function () {
                    if (xhr.readyState === 4 && xhr.status === 200) {
                        refresh()
                    }
                    if (xhr.readyState === 4 && xhr.status !== 200) {
                        console.error('Failed!')
                    }
                }
                xhr.open('POST', '/mode', true)
                xhr.setRequestHeader('Content-Type', 'application/json')
                xhr.send(data)
            }

            function getToggleStatus(index) {
                var xhr = new XMLHttpRequest()
                xhr.open('GET', '/toggle?index=' + index, true)
                xhr.onreadystatechange = function () {
                    if (xhr.readyState === 4 && xhr.status === 200) {
                        var response = xhr.responseText
                        var jsonData = JSON.parse(response)

                        console.log(jsonData, jsonData.data)

                        if (jsonData.data && jsonData.data == 1) {
                            document.getElementById(
                                'switch' + index
                            ).checked = true
                        } else {
                            document.getElementById(
                                'switch' + index
                            ).checked = false
                        }
                    }
                }
                xhr.send()
            }

            function getMode(index) {
                var xhr = new XMLHttpRequest()
                xhr.open('GET', '/mode?index=' + index, true)
                xhr.onreadystatechange = function () {
                    if (xhr.readyState === 4 && xhr.status === 200) {
                        var response = xhr.responseText
                        var jsonData = JSON.parse(response)

                        console.log(jsonData, jsonData.data)

                        if (jsonData.data && jsonData.data == 1) {
                            document.getElementById(
                                'switchauto' + index
                            ).checked = true
                        } else {
                            document.getElementById(
                                'switchauto' + index
                            ).checked = false
                        }
                    }
                }
                xhr.send()
            }

            function refresh() {
                getToggleStatus(0)
                getToggleStatus(1)
                getMode(0)
                getMode(1)
            }

            window.onload = function () {
                refresh()
            }
        </script>
    </body>
</html>

)***";

    _webServer.sendHeader("Connection", "close");
    _webServer.send(200, "text/html", indexHTMLString);
}

void SmartSwitchManager::handleGetToggle()
{
    String indexString = _webServer.arg("index");
    int index = std::stoi(indexString.c_str());

    int toggleStatus = getToggleStatus(index);

    String jsonData = "{";
    jsonData += R"***(
        "data":
    )***";

    jsonData += String(toggleStatus);
    jsonData += "}";

    _webServer.sendHeader("Content-Type", "application/json");
    _webServer.send(200, "application/json", jsonData);
}

void SmartSwitchManager::handleGeteMode()
{
    String indexString = _webServer.arg("index");
    int index = std::stoi(indexString.c_str());

    int mode = getMode(index);

    String jsonData = "{";
    jsonData += R"***(
        "data":
    )***";

    jsonData += String(mode);
    jsonData += "}";

    _webServer.sendHeader("Content-Type", "application/json");
    _webServer.send(200, "application/json", jsonData);
}

void SmartSwitchManager::handleChangeToggle()
{
    String message = _webServer.arg("plain");

    DynamicJsonDocument json(1024);
    deserializeJson(json, message);

    int index = json["index"];
    String action = json["action"];
    if (action == "on")
    {
        turnOnSwitch(index);
    }
    else if (action == "off")
    {
        turnOffSwitch(index);
    }
    else
    {
        toggleSwitch(index);
    }

    setMode(index, NORMAL_MODE);

    String jsonData = R"***(
        {
            "success": 1
        }
    )***";
    _webServer.sendHeader("Content-Type", "application/json");
    _webServer.send(200, "application/json", jsonData);
}

void SmartSwitchManager::handleChangeMode()
{
    String message = _webServer.arg("plain");

    DynamicJsonDocument json(1024);
    deserializeJson(json, message);

    int index = json["index"];
    int mode = json["mode"];
    if (mode == 0)
    {
        setMode(index, NORMAL_MODE);
    }
    else if (mode == 1)
    {
        setMode(index, REACTIVE_MODE);
    }

    String jsonData = R"***(
        {
            "success": 1
        }
    )***";
    _webServer.sendHeader("Content-Type", "application/json");
    _webServer.send(200, "application/json", jsonData);
}

void SmartSwitchManager::loop()
{
    _webServer.handleClient();

    for (SmartSwitch *smartswitch : _switches)
    {
        if (smartswitch->getMode() == REACTIVE_MODE)
        {
            if (_sr501 != nullptr && _sr501->getStatus())
            {
                smartswitch->turnOn();
            }
            else
            {
                smartswitch->turnOff();
            }
        }
    }
}
