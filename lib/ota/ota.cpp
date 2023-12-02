#include <Update.h>
#include "ota.h"

OTAService::OTAService(int serverPort) : _webServer(serverPort)
{
}

OTAService::~OTAService()
{
    _webServer.stop();
}

void OTAService::setup()
{
    std::function<void(void)> handleIndex = std::bind(&OTAService ::handleIndex, this);
    std::function<void(void)> handleUpdate = std::bind(&OTAService ::handleUpdate, this);
    std::function<void(void)> handleDoUpdate = std::bind(&OTAService ::handleDoUpdate, this);

    _webServer.on(
        "/",
        HTTP_GET,
        handleIndex);

    _webServer.on(
        "/update",
        HTTP_POST,
        handleUpdate,
        handleDoUpdate);

    _webServer.begin();
}

void OTAService::loop()
{
    _webServer.handleClient();
}

void OTAService ::handleIndex()
{
    String indexHTMLString = R"***(
<!DOCTYPE html>
<html>
    <head>
        <title>OTA Server</title>
        <style>
            body {
                background: #333;
                font-family: sans-serif;
                font-size: 14px;
                color: #777;
            }
            #form {
                background: #fff;
                max-width: 258px;
                margin: 75px auto;
                padding: 30px;
                border-radius: 5px;
                text-align: center;
            }
            #file-input,
            input {
                width: 100%;
                height: 44px;
                border-radius: 4px;
                margin: 10px auto;
                font-size: 15px;
            }
            input {
                background: #f1f1f1;
                border: 0;
                padding: 0 15px;
            }
            #file-input {
                font-weight: bold;
                padding: 0;
                border: 1px solid #ddd;
                line-height: 44px;
                display: block;
                cursor: pointer;
                text-align: center;
            }
            #bar,
            #prgbar {
                background-color: #f1f1f1;
                border-radius: 10px;
            }
            #bar {
                background-color: #333;
                width: 0%;
                height: 10px;
            }
            .btn {
                background: #333;
                color: #fff;
                cursor: pointer;
            }
        </style>
    </head>
    <body>
        <div id="form">
            <input
                type="file"
                name="update"
                id="file"
                onchange="sub(this);"
                style="display: none"
            />
            <label id="file-input" for="file"> Choose file...</label>
            <input
                type="submit"
                class="btn"
                onclick="updateFile();"
                value="Update"
            />
            <br />
            <br />
            <div id="prg"></div>
            <br />
            <div id="prgbar">
                <div id="bar"></div>
            </div>
            <br />
        </div>
        <script>
            function updateFile() {
                var xhr = new XMLHttpRequest()
                var formData = new FormData()
                var fileInput = document.getElementById('file')

                console.log(fileInput.files)

                formData.append('file', fileInput.files[0])

                xhr.upload.addEventListener(
                    'progress',
                    function (event) {
                        if (event.lengthComputable) {
                            var percentComplete =
                                (event.loaded / event.total) * 100
                            document.getElementById('prg').innerHTML =
                                'Upload progress: ' +
                                percentComplete.toFixed(2) +
                                '%'
                            document.getElementById('bar').style.width =
                                percentComplete.toFixed(2) + '%'
                        }
                    },
                    false
                )

                xhr.onreadystatechange = function () {
                    if (xhr.readyState === 4 && xhr.status === 200) {
                        console.log('File updated successfully')
                    }
                }

                xhr.open('POST', '/update', true)
                xhr.send(formData)
            }

            function sub(obj) {
                var fileName = obj.value.split('\\')
                document.getElementById('file-input').innerHTML =
                    '   ' + fileName[fileName.length - 1]
            }
        </script>
    </body>
</html>
)***";
    _webServer.sendHeader("Connection", "close");
    _webServer.send(200, "text/html", indexHTMLString);
}

void OTAService ::handleUpdate()
{
    _webServer.sendHeader("Connection", "close");
    _webServer.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
}

void OTAService ::handleDoUpdate()
{
    HTTPUpload &upload = _webServer.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN))
        { // start with max available size
            Update.printError(Serial);
        }
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        /* flashing firmware to ESP*/
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
        {
            Update.printError(Serial);
        }
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (Update.end(true))
        { // true to set the size to the current progress
            Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        }
        else
        {
            Update.printError(Serial);
        }
    }
}
