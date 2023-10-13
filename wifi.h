/*
 */

#ifndef _WIFI_H_
#define _WIFI_H_

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#define WIFI_TIMEOUT_S    30          /* Max WiFI waiting connection time (in seconds) */
#define SERVER_TIMEOUT_S  10          /* Max response time waiting for server response */

extern String ssid[];
extern String password[];
extern String hostName;
extern String ap_ssid;
extern String ap_password;
extern String serverName;
extern String serverPath;
extern String serverUser;
extern WiFiClient client;

wifi_mode_t wifi_setup(bool configExist);

#endif _WIFI_H_
