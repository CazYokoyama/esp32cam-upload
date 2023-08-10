/*
 */

#include "wifi.h"

String ssid = "YOUR_SSID";
String password = "YOUR_PASS";

String serverName = "1.2.3.4";
String serverPath = "/upload";
int serverPort = 8080;
String hostName = "esp32cam";
String ap_ssid = hostName;
String ap_password = "123456789";

#define MAX_RETRY 30

WiFiClient client;

wifi_mode_t
wifi_setup(bool configExist)
{
    if (configExist) {
        WiFi.mode(WIFI_STA);
        Serial.print("Connecting to "); Serial.println(ssid);
        WiFi.begin(ssid.c_str(), password.c_str());
        for (int retry = 0; retry++ < MAX_RETRY; ) {
            if (WiFi.status() == WL_CONNECTED)
                break;
            Serial.print(".");
            delay(500);
        }
        Serial.println();
        if (WiFi.status() == WL_CONNECTED) {
            Serial.print("ESP32-CAM IP Address: ");
            Serial.println(WiFi.localIP());
            return  WIFI_MODE_STA;
        }
        Serial.print("can't connect: "); Serial.println(ssid);
    }

    /* start AP */
    WiFi.mode(WIFI_AP);
    WiFi.softAP(hostName.c_str());
    WiFi.softAP(ap_ssid.c_str(), ap_password.c_str());
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: "); Serial.println(IP);
    return WIFI_MODE_AP;
}
