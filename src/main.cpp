#include "main.h"
#include <WiFi.h>
#include <WiFiAP.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Freenove_WS2812_Lib_for_ESP32.h>

#include "ota.h"
#include "sr501.h"
#include "wifimanager.h"
#include "smartswitchmanager.h"

WifiManager wifiManager((char *)"smartlight", &WiFi, &MDNS, &Serial, 8081);
OTAService ota(8080);
SR501Service sr501(1, 1);

SmartSwitch ss1(2);
SmartSwitch ss2(21);
SmartSwitchManager smartswitchManager({&ss1, &ss2}, 80, &sr501);

Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(1, 8, 0, TYPE_RGB);

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