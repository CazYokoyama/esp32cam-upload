/*
 */

#include "wifi.h"

String ssid[] = {
  "YOUR_SSID0",
  "YOUR_SSID1"
};
String password[] = {
  "YOUR_PASS0",
  "YOUR_PASS1"
};

String serverName = "1.2.3.4";
String serverPath = "/upload";
int serverPort = 8080;
String hostName = "esp32cam";
String ap_ssid = hostName;
String ap_password = "123456789";
WiFiMulti *wifiMulti;

#define MAX_RETRY 30

WiFiClient client;

wifi_mode_t
wifi_setup(bool configExist)
{
    if (configExist) {
        WiFi.mode(WIFI_STA);
        Serial.print("Connecting to "); Serial.println(ssid[0]);
        wifiMulti = new WiFiMulti();
        for (int i = 0; i < sizeof(ssid)/sizeof(ssid[0]); i++) {
            if (ssid[i] != "")
                wifiMulti->addAP(ssid[i].c_str(), password[i].c_str());
        }
        for (int retry = 0; retry++ < MAX_RETRY; ) {
            if (wifiMulti->run() == WL_CONNECTED)
                break;
            Serial.print(".");
            delay(500);
        }
        Serial.println();

        if (wifiMulti->run() == WL_CONNECTED) {
            Serial.print("Connect "); Serial.print(WiFi.SSID());
            Serial.print(" at "); Serial.println(WiFi.localIP());
            return  WIFI_MODE_STA;
        }
        Serial.println("can't connect any of APs");
    }

    /* start AP */
    WiFi.mode(WIFI_AP);
    WiFi.softAP(hostName.c_str());
    WiFi.softAP(ap_ssid.c_str(), ap_password.c_str());
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: "); Serial.println(IP);
    return WIFI_MODE_AP;
}
