#include "main.h"
#include <WiFi.h>
#include <WiFiAP.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Freenove_WS2812_Lib_for_ESP32.h>
#include <NTPClient.h>
#include <TimeLib.h>

#include "ota.h"
#include "sr501.h"
#include "wifimanager.h"
#include "smartswitchmanager.h"

WifiManager wifiManager((char *)"smartlight", &WiFi, &MDNS, &Serial, 8081);
OTAService ota(8080);
SR501Service sr501(1);

SmartSwitch ss1(2);
SmartSwitch ss2(21);
SmartSwitchManager smartswitchManager({&ss1, &ss2}, 80, &sr501);

Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(1, 8, 0, TYPE_RGB);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void setup()
{
    Serial.begin(115200);

    // wifiManager
    wifiManager.setup();

    // ota
    ota.setup();

    // smartswitchManager
    smartswitchManager.setup();

    strip.begin();
    strip.setBrightness(20);

    // 初始化NTP客户端
    timeClient.begin();
    timeClient.setTimeOffset(28800); // 设置时区（单位：秒），东8区为28800秒
}

void loop()
{
    wifiManager.loop();

    sr501.loop();

    ota.loop();

    smartswitchManager.loop();

    for (int j = 0; j < 255; j += 2)
    {
        strip.setLedColorData(0, strip.Wheel((256 / 1 + j) & 255));

        strip.show();
        delay(10);
    }

    // 更新时间
    timeClient.update();

    // 获取当前时间
    int currentHour = hour();
    int currentMinute = minute();
    int currentSecond = second();

    // 执行任务
    if (currentHour == 1 && currentMinute == 0 && currentSecond == 0)
    {
        smartswitchManager.turnOffAllSwitch();
        smartswitchManager.turnOnSwitch(1);
        smartswitchManager.setMode(1, REACTIVE_MODE);
    }
    else if (currentHour == 7 && currentMinute == 10 && currentSecond == 0)
    {
        smartswitchManager.turnOnAllSwitch();
    }
    else if (currentHour == 9 && currentMinute == 0 && currentSecond == 0)
    {
        smartswitchManager.turnOffAllSwitch();
    }
}