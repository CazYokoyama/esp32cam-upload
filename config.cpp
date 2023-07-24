/*
 */

#include "spiffs.h"
#include "config.h"

#define JSON_MAX 400
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

  ssid = obj["ssid"].as<String>();
  password = obj["password"].as<String>();
  hostName = obj["hostName"].as<String>();
  ap_ssid = obj["ap_ssid"].as<String>();
  ap_password = obj["ap_password"].as<String>();
  serverName = obj["serverName"].as<String>();
  serverPath = obj["serverPath"].as<String>();
  serverPort = obj["serverPort"];
  time_to_sleep_s = obj["time_to_sleep_s"];
  time_to_reboot = obj["time_to_reboot"];

  return true;
}

int
save_config()
{
  StaticJsonDocument<JSON_MAX> obj;

  obj["ssid"] = ssid;
  obj["password"] = password;
  obj["hostName"] = hostName;
  obj["ap_ssid"] = ap_ssid;
  obj["ap_password"] = ap_password;
  obj["serverName"] = serverName;
  obj["serverPath"] = serverPath;
  obj["serverPort"] = serverPort;
  obj["time_to_sleep_s"] = time_to_sleep_s;
  obj["time_to_reboot"] = time_to_reboot;

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
