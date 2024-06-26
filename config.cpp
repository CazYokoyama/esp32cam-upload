/*
 */

#include "spiffs.h"
#include "config.h"
#include "camera.h"

#define JSON_MAX 600
#define CONFIG_FILE_NAME "/config.json"

String
show_config()
{
  File file = SPIFFS.open(CONFIG_FILE_NAME);
  if (!file) {
     Serial.print("can't open file for writing: ");
     Serial.println(CONFIG_FILE_NAME);
     return "";
  }

  StaticJsonDocument<JSON_MAX> obj;
  DeserializationError error = deserializeJson(obj, file);
  if (error) {
     Serial.print("can't read json from ");
     Serial.print(CONFIG_FILE_NAME);
     Serial.print(": ");
     Serial.println(error.c_str());
     file.close();
     return "";
  }
  file.close();

  String output;
  serializeJsonPretty(obj, output);

  return output;
}

bool
read_config()
{
  File file = SPIFFS.open(CONFIG_FILE_NAME);
  if (!file){
     Serial.print("can't open file for writing: ");
     Serial.println(CONFIG_FILE_NAME);
     return false;
  }

  StaticJsonDocument<JSON_MAX> obj;
  DeserializationError error = deserializeJson(obj, file);
  if (error) {
     Serial.print("can't read json from ");
     Serial.print(CONFIG_FILE_NAME);
     Serial.print(": ");
     Serial.println(error.c_str());
     file.close();
     return false;
  }
  file.close();

  if (obj.containsKey(F("ssid"))) {
    ssid[0] = obj["ssid"][0].as<String>();
    ssid[1] = obj["ssid"][1].as<String>();
  }
  if (obj.containsKey(F("password"))) {
    password[0] = obj["password"][0].as<String>();
    password[1] = obj["password"][1].as<String>();
  }
  if (obj.containsKey(F("hostName")))
    hostName = obj["hostName"].as<String>();
  if (obj.containsKey(F("ap_ssid")))
    ap_ssid = obj["ap_ssid"].as<String>();
  if (obj.containsKey(F("ap_password")))
    ap_password = obj["ap_password"].as<String>();
  if (obj.containsKey(F("serverName")))
    serverName = obj["serverName"].as<String>();
  if (obj.containsKey(F("serverPath")))
    serverPath = obj["serverPath"].as<String>();
  if (obj.containsKey(F("serverUser")))
    serverUser = obj["serverUser"].as<String>();
  if (obj.containsKey(F("time_to_sleep_s")))
    time_to_sleep_s = obj["time_to_sleep_s"];
  if (obj.containsKey(F("time_to_reboot")))
    time_to_reboot = obj["time_to_reboot"];
  if (obj.containsKey(F("sleep_in_night")))
    sleep_in_night = obj["sleep_in_night"];
  if (obj.containsKey(F("day_night_threshold")))
    day_night_threshold = obj["day_night_threshold"];

  return true;
}

int
save_config()
{
  StaticJsonDocument<JSON_MAX> obj;

  obj["ssid"][0] = ssid[0];
  obj["ssid"][1] = ssid[1];
  obj["password"][0] = password[0];
  obj["password"][1] = password[1];
  obj["hostName"] = hostName;
  obj["ap_ssid"] = ap_ssid;
  obj["ap_password"] = ap_password;
  obj["serverName"] = serverName;
  obj["serverPath"] = serverPath;
  obj["serverUser"] = serverUser;
  obj["time_to_sleep_s"] = time_to_sleep_s;
  obj["time_to_reboot"] = time_to_reboot;
  obj["sleep_in_night"] = sleep_in_night;
  obj["day_night_threshold"] = day_night_threshold;

  File file = SPIFFS.open(CONFIG_FILE_NAME, FILE_WRITE);
  if (!file){
     Serial.print("can't open file for writing: ");
     Serial.println(CONFIG_FILE_NAME);
     return 0;
  }
  int res = serializeJson(obj, file);
  if (res <= 0) {
     Serial.print("can't write json to ");
     Serial.println(CONFIG_FILE_NAME);
  }
  file.close();

  return res;
}

void
config_setup()
{
  spiffs_setup();
}
