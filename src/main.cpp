#include "main.h"
#include <WiFi.h>
#include <WiFiAP.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Freenove_WS2812_Lib_for_ESP32.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#include "ota.h"
#include "sr501.h"
#include "wifimanager.h"
#include "smartswitchmanager.h"

WifiManager wifiManager((char *)"smartlight", &WiFi, &MDNS, &Serial, 8081);
OTAService ota(8080);
SR501Service sr501(A1);

SmartSwitch ss1(2);
SmartSwitch ss2(21);
SmartSwitchManager smartswitchManager({&ss1, &ss2}, 80, &sr501);

Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(1, 8, 0, TYPE_RGB);

#ifdef NTP_SERVER
static const char *ntpServer = EXPAND_AND_STRINGIFY(NTP_SERVER);
#else
static const char *ntpServer = "pool.ntp.org";
#endif

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, 28800, 1000 * 60 * 60);

TaskHandle_t taskHandle;
void taskFunction(void *parameter);

void setup()
{
    Serial.begin(115200);

    // wifiManager
    wifiManager.setup();

    // NTP
    timeClient.begin();

    // ota
    ota.setup();

    // smartswitchManager
    smartswitchManager.setup();

    // RGBLed
    strip.begin();
    strip.setBrightness(20);

    xTaskCreate(taskFunction, "Task", 4096, NULL, 1, &taskHandle);
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
}

void taskFunction(void *parameter)
{
    while (true)
    {
        timeClient.update();

        int currentHour = timeClient.getHours();
        int currentMinute = timeClient.getMinutes();
        int currentSecond = timeClient.getSeconds();
        Serial.println(timeClient.getFormattedTime());

        // 获取时间戳
        unsigned long epochTime = timeClient.getEpochTime();
        // 设置系统时间
        struct timeval tv;
        tv.tv_sec = epochTime;
        tv.tv_usec = 0;
        settimeofday(&tv, NULL);

        struct tm timeInfo;

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
        delay(600);
    }
}
