/*
ESP32-CAM DEEP SLEEP TIME LAPSE CAMERA  PROOF-OF-CONCEPT
PICTURE STORED IN SD CARD AND THEN UPLOADED VIA WIFI
TO A SERVER USING HTTP MULTIPART POST
*/

#include <Arduino.h>
#include <WiFi.h>
#include "wifi.h"
#include "config.h"
#include "spiffs.h"
#include "web.h"
#include "camera.h"

#define FLASHLED_GPIO_NUM 4

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

void setup() {
  Serial.begin(115200);

  // Turns off the ESP32-CAM white on-board LED (flash)
  pinMode(FLASHLED_GPIO_NUM, OUTPUT);
  digitalWrite(FLASHLED_GPIO_NUM, LOW);

  print_wakeup_reason();
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED)
    photo_prev = 0;
  else {
    if (get_average_brightness() < day_night_threshold) { /* night */
      Serial.print("sleep for "); Serial.print(time_to_sleep_s);
      Serial.println(" sec."); Serial.flush();
      esp_sleep_enable_timer_wakeup(time_to_sleep_s * uS_TO_S_FACTOR);
       esp_deep_sleep_start();
    }
  }

  camera_setup();

  config_setup();
  bool configExist = read_config();
  wifimode = wifi_setup(configExist);
  web_setup();
}

void loop() {
    unsigned long photo_current = millis();

    switch (wifimode) {
    case WIFI_STA:
        if (photo_prev >= time_to_reboot * mS_TO_S_FACTOR) {
            Serial.printf("%s() %d: %lu %lu\n", __func__, __LINE__,
                          photo_prev, photo_current);
            uploadPhoto();
            photo_prev += photo_current + time_to_sleep_s;
            Serial.print("sleep for "); Serial.print(time_to_sleep_s);
            Serial.println(" sec."); Serial.flush();
            esp_sleep_enable_timer_wakeup(time_to_sleep_s * uS_TO_S_FACTOR);
            esp_deep_sleep_start();
        } else {
            /* don't sleep for time_to_reboot sec since boot */
            if (photo_prev == 0 ||
                (photo_current - photo_prev) > time_to_sleep_s * mS_TO_S_FACTOR) {
                // Turns on the ESP32-CAM white on-board LED (flash)
                digitalWrite(FLASHLED_GPIO_NUM, HIGH);
		delay(1);
                // Turns off the ESP32-CAM white on-board LED (flash)
                digitalWrite(FLASHLED_GPIO_NUM, LOW);

                uploadPhoto();
                photo_prev = photo_current;
            }
            web_loop();
        }
        break;
    case WIFI_AP:
        if (photo_current >= time_to_reboot * mS_TO_S_FACTOR)
          ESP.restart();
        web_loop();
        break;
    default:
        delay(time_to_reboot * mS_TO_S_FACTOR);
        ESP.restart();
        break;
    }
}
