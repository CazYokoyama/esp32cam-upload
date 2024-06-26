/*
 */

#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <Arduino.h>
#include "esp_camera.h"

// CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define DEBUGLED_GPIO_NUM 33

extern u8_t day_night_threshold;
extern volatile bool sendPhoto;

void camera_setup();
void camera_deinit();
int get_average_brightness();
camera_fb_t *capturePhoto();
void releasePhoto(camera_fb_t *fb);
void uploadPhoto();

#endif _CAMERA_H_
