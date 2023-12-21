#include "wifimanager.h"
#include <ArduinoJson.h>
#include "stringify.h"

static const int ssidEEPROMAddress = 0x100;
static const int passwordEEPROMAddress = 0x120;

#ifdef AP_SSID
static const char *apSsid = EXPAND_AND_STRINGIFY(AP_SSID);
#else
static const char *apSsid = "esp32-ap";
#endif

#ifdef AP_PASSWORD
static const char *apPassword = EXPAND_AND_STRINGIFY(AP_PASSWORD);
#else
static const char *apPassword = "12345678";
#endif

WifiManager::WifiManager(
    char *hostName,
    WiFiClass *wifi,
    MDNSResponder *mdns,
    HWCDC *serial,
    int serverPort)
    : _preferences(),
      _webServer(serverPort)
{
    _hostName = hostName;
    _wifi = wifi;
    _mdns = mdns;
    _serial = serial;
}

WifiManager::~WifiManager()
{
    _webServer.stop();
}

void WifiManager::setup()
{
    connectToWiFi();

    std::function<void(void)>
        handleIndex = std::bind(&WifiManager::handleIndex, this);
    std::function<void(void)> handleSavePassword = std::bind(&WifiManager::handleSavePassword, this);
    std::function<void(void)> handleClearPassword = std::bind(&WifiManager::handleClearPassword, this);

    _webServer.on(
        "/",
        HTTP_GET,
        handleIndex);
    _webServer.begin();

    _webServer.on(
        "/save",
        HTTP_POST,
        handleSavePassword);

    _webServer.on(
        "/clear",
        HTTP_POST,
        handleClearPassword);

    _webServer.begin();
}

void WifiManager::loop()
{

    if (_wifi->status() != WL_CONNECTED)
    {
        Serial.println("Retry connect...");
        connectToWiFi();
    }

    _webServer.handleClient();
}

void WifiManager::handleIndex()
{
    String indexHTMLString = R"***(
<!DOCTYPE html>

<html>
    <head>
        <title>Wifi Manager</title>
        <style>
            body {
                background: #333;
                font-family: sans-serif;
                font-size: 14px;
                color: #777;
            }
            #form {
                background: #fff;
                max-width: 320px;
                margin: 75px auto;
                padding: 30px;
                border-radius: 5px;
                text-align: center;
            }
            #form input {
                width: 260px;
                height: 44px;
                border-radius: 4px;
                margin: 10px auto;
                font-size: 15px;
                padding: 0 15px;
            }
            input {
                background: #f1f1f1;
                border: 0;
            }
            #form .btn {
                width: 290px;
                background: #333;
                color: #fff;
                cursor: pointer;
            }

            #form .reset {
                background: #900;
            }
        </style>
    </head>
    <body>
        <div id="form">
            <form>
                <input type="text" name="ssid" id="ssid" placeholder="SSID" />
                <input
                    type="password"
                    name="password"
                    id="password"
                    placeholder="Password"
                    autocomplete
                />
                <input
                    type="button"
                    class="btn"
                    onclick="savePassword();"
                    value="Save"
                />
                <input
                    type="reset"
                    class="btn reset"
                    onclick="clearPassword();"
                    value="Clear"
                />
            </form>
        </div>
        <script>
            function savePassword() {
                var xhr = new XMLHttpRequest()
                var ssid = document.getElementById('ssid').value
                var password = document.getElementById('password').value
                var data = JSON.stringify({ ssid, password })
                xhr.onreadystatechange = function () {
                    if (xhr.readyState === 4 && xhr.status === 200) {
                        alert('Successfully saved. Please restart the device.')
                    }
                    if (xhr.readyState === 4 && xhr.status !== 200) {
                        alert('Failed!')
                    }
                }
                xhr.open('POST', '/save', true)
                xhr.setRequestHeader('Content-Type', 'application/json')
                xhr.send(data)
            }

            function clearPassword() {
                var xhr = new XMLHttpRequest()
                var ssid = document.getElementById('ssid').value
                var password = document.getElementById('password').value
                var data = JSON.stringify({ ssid, password })
                xhr.onreadystatechange = function () {
                    if (xhr.readyState === 4 && xhr.status === 200) {
                        alert(
                            'Successfully cleared. Please restart the device.'
                        )
                    }
                    if (xhr.readyState === 4 && xhr.status !== 200) {
                        alert('Failed!')
                    }
                }
                xhr.open('POST', '/clear', true)
                xhr.setRequestHeader('Content-Type', 'application/json')
                xhr.send(data)
            }
        </script>
    </body>
</html>
)***";
    _webServer.sendHeader("Connection", "close");
    _webServer.send(200, "text/html", indexHTMLString);
}

void WifiManager::handleSavePassword()
{

    String message = _webServer.arg("plain");

    DynamicJsonDocument json(1024);
    deserializeJson(json, message);

    String ssid = json["ssid"];
    String password = json["password"];

    String jsonData = R"***(
        {
            "success": 1
        }
    )***";

    _preferences.begin("wifimanage", false);
    _preferences.putString("wifi_ssid", ssid);
    _preferences.putString("wifi_password", password);

    _preferences.end();
    _serial->printf("wifi_ssid: %s \r\n", ssid);
    _serial->printf("wifi_password: %s \r\n", password);

    _webServer.sendHeader("Content-Type", "application/json");
    _webServer.send(200, "application/json", jsonData);
}

void WifiManager::handleClearPassword()
{
    String jsonData = R"***(
        {
            "success": 1
        }
    )***";

    _preferences.begin("wifimanage", false);
    _preferences.putString("wifi_ssid", "");
    _preferences.putString("wifi_password", "");

    _preferences.end();

    _serial->printf("wifi_ssid and wifi_password cleared\r\n");

    _webServer.sendHeader("Content-Type", "application/json");
    _webServer.send(200, "application/json", jsonData);
}

void WifiManager::connectToWiFi()
{
    int trySTA = WifiManager::trySTA();
    _serial->printf("trySTA: ");
    _serial->println(trySTA);

    if (!trySTA)
    {
        if (!_wifi->softAP(apSsid, apPassword, 11))
        {
            log_e("Soft AP creation failed.");
            while (1)
                ;
        }

        _serial->print("IP Address: ");
        _serial->println(_wifi->softAPIP());
    }
}

int WifiManager::trySTA()
{
    int tried = 0;

    _preferences.begin("wifimanage", true);
    String ssid = _preferences.getString("wifi_ssid", "");
    String password = _preferences.getString("wifi_password", "");
    _serial->printf("wifi_ssid: %s \r\n", ssid);
    _serial->printf("wifi_password: %s \r\n", password);

    _preferences.end();

    _wifi->begin(ssid, password);
    _wifi->setHostname(_hostName);

    while (
        _wifi->status() == WL_NO_SHIELD ||
        _wifi->status() == WL_SCAN_COMPLETED ||
        _wifi->status() == WL_IDLE_STATUS ||
        _wifi->status() == WL_DISCONNECTED)
    {
        _serial->println("Connecting to WiFi...");
        _serial->printf("Wifi Status: %d, IP Address: %s \r\n", _wifi->status(), _wifi->localIP().toString());
        delay(300);
        tried++;
        if (tried > 50)
        {
            return 0;
        }
    }

    if (_wifi->status() == WL_CONNECTED)
    {
        _serial->printf("Wifi Status: %d, IP Address: %s \r\n", _wifi->status(), _wifi->localIP().toString());
        _serial->println("Connected to WiFi");

        if (!_mdns->begin(_hostName))
        {
            _serial->println("Error setting up MDNS responder");
        }
        return 1;
    }

    return 0;
}
