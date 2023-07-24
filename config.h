/*
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <ArduinoJson.h>
#include "wifi.h"
#include "camera.h"

extern int time_to_sleep_s;
extern int time_to_reboot;

String show_config();
bool read_config();
int save_config();
void config_setup();

#endif _CONFIG_H_
