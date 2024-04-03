/*
 */

#include "wifi.h"
#include "camera.h"

u8_t day_night_threshold = 20; /* 20% may be an appropriate value */
volatile bool sendPhoto = false;

static void
camera_common_config(camera_config_t *config)
{
  config->ledc_channel = LEDC_CHANNEL_0;
  config->ledc_timer = LEDC_TIMER_0;
  config->pin_d0 = Y2_GPIO_NUM;
  config->pin_d1 = Y3_GPIO_NUM;
  config->pin_d2 = Y4_GPIO_NUM;
  config->pin_d3 = Y5_GPIO_NUM;
  config->pin_d4 = Y6_GPIO_NUM;
  config->pin_d5 = Y7_GPIO_NUM;
  config->pin_d6 = Y8_GPIO_NUM;
  config->pin_d7 = Y9_GPIO_NUM;
  config->pin_xclk = XCLK_GPIO_NUM;
  config->pin_pclk = PCLK_GPIO_NUM;
  config->pin_vsync = VSYNC_GPIO_NUM;
  config->pin_href = HREF_GPIO_NUM;
  config->pin_sscb_sda = SIOD_GPIO_NUM;
  config->pin_sscb_scl = SIOC_GPIO_NUM;
  config->pin_pwdn = PWDN_GPIO_NUM;
  config->pin_reset = RESET_GPIO_NUM;
  config->xclk_freq_hz = 20000000;
}

void
camera_setup()
{
  camera_config_t config;

  camera_common_config(&config);
  config.pixel_format = PIXFORMAT_JPEG;

  // init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_SXGA;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    Serial.println("reboot");
    Serial.flush();
    ESP.restart();
  }
}

void
camera_deinit()
{
  digitalWrite(PWDN_GPIO_NUM, HIGH);
  esp_camera_deinit();
}

/*
 * return 0-100%
 */
int
get_average_brightness()
{
  camera_config_t config;

  camera_common_config(&config);
  config.pixel_format = PIXFORMAT_GRAYSCALE;
  config.frame_size = FRAMESIZE_QVGA;
  config.fb_count = 1;
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return err;
  }

  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return ESP_FAIL;
  }

  const char *data = (const char *)fb->buf;
  size_t size = fb->len;
  unsigned long b = 0;
  for (int t = 0; t++ < fb->len; )
    b += *data++;
  b /= fb->len; /* average */
  b = (b * 100) / 255; /* convert to percent */
  Serial.printf("%s() %d: %d%%\n", __func__, __LINE__,
		b);

  esp_camera_fb_return(fb);
  esp_camera_deinit();

  return b;
}

camera_fb_t *
capturePhoto()
{
  camera_fb_t *pbuff = NULL;

  /* discard an image once to adjust white ballance */
  // take a frame and discard
  pbuff = esp_camera_fb_get();
  if (!pbuff)
    return pbuff;
  esp_camera_fb_return(pbuff);

  pbuff = esp_camera_fb_get();
  if (!pbuff)
    return pbuff;
  esp_camera_fb_return(pbuff);

  pbuff = esp_camera_fb_get();

  return pbuff;
}

void
releasePhoto(camera_fb_t *fb)
{
  esp_camera_fb_return(fb);
}

void
uploadPhoto()
{

  sendPhoto = true;

  while (sendPhoto)
    delay(1 * 1000); /* check whether finish uploading every 1 sec */
}
