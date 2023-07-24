/*
 */

#ifndef _WIFI_H_
#define _WIFI_H_

#include <Arduino.h>
#include <WiFi.h>

#define WIFI_TIMEOUT_S    30          /* Max WiFI waiting connection time (in seconds) */
#define SERVER_TIMEOUT_S  10          /* Max response time waiting for server response */

extern String ssid;
extern String password;
extern String serverName;
extern String serverPath;
extern int serverPort;
extern WiFiClient client;

void wifi_setup();

#endif _WIFI_H_
