#include <FS.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <Update.h>
#include "web.h"
#include "spiffs.h"
#include "git-version.h"

// SKETCH BEGIN
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");

static const char upload_html[] PROGMEM = "\
<html>\
  <head>\
    <meta http-equiv='Content-Type' content='text/html; charset=utf-8'>\
  </head>\
  <body>\
    <h1>ESP32CAM periodic upload</h1>\
    <h3>Version %s</h3>\
    <div class = 'upload'>\
      <form method = 'POST' action = '/doUpload' enctype='multipart/form-data'>\
        <input type='file' name='data'/>\
        <input type='submit' name='upload' value='Upload' title = 'Upload Files'>\
      </form>\
    </div>\
    <div class = 'update'>\
      <form method = 'POST' action = '/update' enctype='multipart/form-data'>\
        <input type='file' name='data'/>\
        <input type='submit' name='update' value='Update' title = 'Update Firmware'>\
      </form>\
    </div>\
    <td><a href='format' class='button_format' >Format SPIFFS</a></td>\
    <td><a href='reboot' class='button_reboot' >Reboot</a></td>\
  </body>\
</html>\
";

void
handleUpload(AsyncWebServerRequest *request, String filename, size_t index,
	     uint8_t *data, size_t len, bool final)
{
    if (index == 0) {
        Serial.print("open: "); Serial.println(filename); Serial.flush();
        request->_tempFile = SPIFFS.open(("/" + filename).c_str(), "w");
    }
    if (len) {
        Serial.print("write: "); Serial.println(len); Serial.flush();
        // stream the incoming chunk to the opened file
        request->_tempFile.write(data, len);
    }
    if (final) {
        Serial.print("close: "); Serial.println(filename); Serial.flush();
        request->_tempFile.close();
        request->redirect("/");
    }
}

void
web_setup()
{
  MDNS.addService("http","tcp",80);

  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  server.on("/format", HTTP_GET, [](AsyncWebServerRequest *request){
      Serial.printf("format SPIFFS\n");
      formatSPIFFS(SPIFFS);
      request->redirect("/");
  });

  server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Rebooted");
      response->addHeader("Connection", "close");
      request->send(response);
      request->redirect("/");
      Serial.println("Reboot"); Serial.flush();
      ESP.restart();
  });

  size_t size = 1500;
  char *Settings_temp = (char *)malloc(size);
  if (Settings_temp == NULL) {
    Serial.println("can't allocate memory for web ui"); Serial.flush();
    return;
  }
  snprintf(Settings_temp, size, upload_html, GIT_VERSION);
  String html = String(Settings_temp);
  free(Settings_temp);

  server.on("/", HTTP_GET, [html](AsyncWebServerRequest* request) {
    request->send(200, "text/html", html);
  });

  server.on("/doUpload", HTTP_POST, [](AsyncWebServerRequest* request) {}, handleUpload);

// Simple Firmware Update Form
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html",
      "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
  });
  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", Update.hasError() ? "FAIL" : "OK");
    response->addHeader("Connection", "close");
    request->send(response);
  },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (index == 0) {
      Serial.printf("Update Start: %s\n", filename.c_str());
      if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000))
        Update.printError(Serial);
    }
    if (!Update.hasError()) {
      if (Update.write(data, len) != len)
        Update.printError(Serial);
    }
    if (final) {
      if(Update.end(true))
        Serial.printf("Update Success: %uB\n", index + len);
      else
        Update.printError(Serial);
    }
  });

  server.begin();
}

void web_loop()
{
  ws.cleanupClients();
}
