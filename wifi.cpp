/*
 */

#include "wifi.h"

char *ssid = "YOUR_SSID";
char *password = "YOUR_PASS";

String serverName = "1.2.3.4";
String serverPath = "/upload";
int serverPort = 8080;

WiFiClient client;

void
wifi_setup()
{
  int wTimer = WIFI_TIMEOUT_S*1000;
  long wStartTimer = millis();

  WiFi.mode(WIFI_STA);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while ((wStartTimer + wTimer) > millis()) {
      if (WiFi.status() == WL_CONNECTED)
        break;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("connection to WiFI failed");
    Serial.println("Going to sleep now ");
    Serial.flush();
    esp_deep_sleep_start();
  }

  Serial.println();
  Serial.print("ESP32-CAM IP Address: ");
  Serial.println(WiFi.localIP());
}
