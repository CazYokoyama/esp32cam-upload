/*
 */

#include "wifi.h"

String ssid = "YOUR_SSID";
String password = "YOUR_PASS";

String serverName = "1.2.3.4";
String serverPath = "/upload";
int serverPort = 8080;
char *hostName = "esp32cam";
char *ap_ssid = hostName;
char *ap_password = "123456789";

#define MAX_RETRY 30

WiFiClient client;

wifi_mode_t
wifi_setup()
{
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

    /* start AP */
    WiFi.mode(WIFI_AP);
    WiFi.softAP(hostName);
    WiFi.softAP(ap_ssid, ap_password);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: "); Serial.println(IP);
    return WIFI_MODE_AP;
}
