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
int time_to_sleep_s = 60;      /* Time ESP32 will go to sleep (in seconds) */
int time_to_reboot = 10 * 60;  /* Time to reboot on AP mode (in seconds) */

RTC_NOINIT_ATTR static unsigned long photo_prev = 0;
wifi_mode_t wifimode = WIFI_MODE_NULL;

/*
  Method to print the reason by which ESP32
  has been awaken from sleep
 */
void
print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup by ULP program"); break;
    default : Serial.printf("Wakeup by %d\n", wakeup_reason); break;
  }
}

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

  print_wakeup_reason();
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED)
    photo_prev = 0;

  camera_setup();

  config_setup();
  bool configExist = read_config();
  wifimode = wifi_setup(configExist);
}

void loop() {
    unsigned long photo_current;

    switch (wifimode) {
    case WIFI_STA:
        photo_current = millis();
        if (photo_prev >= time_to_reboot * mS_TO_S_FACTOR) {
            uploadPhoto();
            Serial.print("sleep for "); Serial.print(time_to_sleep_s);
            Serial.println(" sec."); Serial.flush();
            esp_sleep_enable_timer_wakeup(time_to_sleep_s * uS_TO_S_FACTOR);
            esp_deep_sleep_start();
        } else {
            /* don't sleep for time_to_reboot sec since boot */
            if ((photo_current - photo_prev) > time_to_sleep_s * mS_TO_S_FACTOR) {
                uploadPhoto();
                photo_prev = photo_current;
            }
        }
        break;
    case WIFI_AP:
        delay(time_to_reboot * mS_TO_S_FACTOR);
        break;
    default:
        delay(time_to_reboot * mS_TO_S_FACTOR);
        ESP.restart();
        break;
    }
}
