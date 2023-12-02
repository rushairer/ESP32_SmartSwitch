#ifndef __INCLUDE_OTA_H_
#define __INCLUDE_OTA_H_
#include <WebServer.h>

class OTAService;

class OTAService
{
public:
    OTAService(int serverPort = 8080);
    virtual ~OTAService();

    void setup();
    void loop();

protected:
    WebServer _webServer;

private:
    void handleIndex();
    void handleUpdate();
    void handleDoUpdate();
};

#endif