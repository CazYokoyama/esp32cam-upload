/*
 */

#include "wifi.h"
#include "camera.h"

void
camera_setup()
{
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
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

  /* discard an image once to adjust white ballance */
  camera_fb_t *pbuff = NULL;
  // take the 1st frame and discard
  pbuff = esp_camera_fb_get();
  if(!pbuff) {
    Serial.println("Camera capture failed");
    delay(1000);
    Serial.println("reboot");
    Serial.flush();
    ESP.restart();
  }
  esp_camera_fb_return(pbuff);
}

String uploadPhoto()
{
  String getAll;
  String getBody;
  camera_fb_t *pbuff = NULL;

  Serial.println("Connecting to server: " + serverName);

  pbuff = esp_camera_fb_get();
  if(!pbuff) {
    Serial.println("Camera capture failed");
    delay(1000);
    Serial.println("reboot");
    Serial.flush();
    ESP.restart();
  }

  if (client.connect(serverName.c_str(), serverPort)) {
    Serial.println("Connection successful!");
    String head = "\
--esp32cam-upload\r\nContent-Disposition: form-data; name=\"imageFile\";\
 filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n\
";
    String tail = "\r\n--esp32cam-upload\r\n";

    uint32_t imageLen = pbuff->len;
    uint32_t extraLen = head.length() + tail.length();
    uint32_t totalLen = imageLen + extraLen;

    client.println("POST " + serverPath + " HTTP/1.1");
    client.println("Host: " + serverName);
    client.println("Content-Length: " + String(totalLen));
    client.println("Content-Type: multipart/form-data; boundary=esp32cam-upload");
    client.println();
    client.print(head);

    uint8_t *fbBuf = pbuff->buf;
    size_t fbLen = pbuff->len;
    for (size_t n = 0; n < fbLen; n = n + 1024) {
      if (n + 1024 < fbLen) {
        client.write(fbBuf, 1024);
        fbBuf += 1024;
      } else if (fbLen % 1024 > 0) {
        size_t remainder = fbLen % 1024;
        client.write(fbBuf, remainder);
      }
    }
    client.print(tail);

    esp_camera_fb_return(pbuff);

    int timoutTimer = SERVER_TIMEOUT_S * 1000;
    long startTimer = millis();
    boolean state = false;

    while ((startTimer + timoutTimer) > millis()) {
      Serial.print(".");
      delay(100);
      while (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (getAll.length() == 0)
            state = true;
          getAll = "";
        } else if (c != '\r')
          getAll += String(c);
        if (state)
          getBody += String(c);
        startTimer = millis();
      }
      if (getBody.length() > 0)
        break;
    }
    Serial.println();
    client.stop();
    Serial.println(getBody);
  } else {
    getBody = "Connection to " + serverName +  " failed.";
    Serial.println(getBody);
  }
  return getBody;
}
