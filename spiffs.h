/*
 */

#ifndef __SPIFFS_H__
#define __SPIFFS_H__

#include "FS.h"
#include "SPIFFS.h"

void listDir(fs::FS &fs, const char *dirname, uint8_t levels);
void deleteFile(fs::FS &fs, const char *path);
void formatSPIFFS(fs::FS &fs);
void spiffs_setup();

#endif __SPIFFS_H__
