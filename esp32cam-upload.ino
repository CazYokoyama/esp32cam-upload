/*
ESP32-CAM DEEP SLEEP TIME LAPSE CAMERA  PROOF-OF-CONCEPT
PICTURE STORED IN SD CARD AND THEN UPLOADED VIA WIFI
TO A SERVER USING HTTP MULTIPART POST
*/

#include <Arduino.h>
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"

String serverEndpoint = "/upload";        // Needs to match upload server endpoint
String keyName = "\"myFile\"";            // Needs to match upload server keyName

#include "wifi.h"
#include "config.h"
#include "spiffs.h"

#define FLASHLED_GPIO_NUM 4
#define RTCLED_GPIO_NUM   GPIO_NUM_4
#define DEBUGLED_GPIO_NUM 33

#include "camera.h"

#define mS_TO_S_FACTOR    1000        /* Conversion factor for mili seconds to seconds */
#define uS_TO_S_FACTOR    1000000ULL  /* Conversion factor for micro seconds to seconds */
int time_to_sleep_s = 60; /* Time ESP32 will go to sleep (in seconds) */

int picNumber = 0;
String sdpath = "/images/";
String picname = "image";
String filext = ".jpg";

void setup() {
  
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  rtc_gpio_hold_dis(RTCLED_GPIO_NUM);
  esp_sleep_enable_timer_wakeup(time_to_sleep_s * uS_TO_S_FACTOR);
  
  // Turn off debug led
  pinMode(DEBUGLED_GPIO_NUM, OUTPUT);
  digitalWrite(DEBUGLED_GPIO_NUM, HIGH);
  // Turns off the ESP32-CAM white on-board LED (flash)
  pinMode(FLASHLED_GPIO_NUM, OUTPUT);
  digitalWrite(FLASHLED_GPIO_NUM, LOW);
  
  Serial.begin(115200);

  camera_setup();

  config_setup();
  read_config();

  wifi_setup();

  uploadPhoto();   
  
  Serial.println("Going to sleep now ");
  Serial.flush(); 
  esp_deep_sleep_start();
  
   
}

void loop() {
  
}
