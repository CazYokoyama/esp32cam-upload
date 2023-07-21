/*
 */

#include "spiffs.h"
#include "config.h"

#define FORMAT_SPIFFS_IF_FAILED true

void
listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if (!root){
        Serial.println("- failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels)
                listDir(fs, file.name(), levels - 1);
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void
deleteFile(fs::FS &fs, const char *path)
{
    Serial.printf("Deleting file: %s\r\n", path);
    if (fs.remove(path))
        Serial.println("- file deleted");
    else
        Serial.println("- delete failed");
}

void
formatSPIFFS(fs::FS &fs)
{
    if (!SPIFFS.format()) {
        Serial.println("SPIFFS Format Failed");
        return;
    }
    Serial.println("SPIFFS Formatted");
}

void
spiffs_setup()
{
    if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    listDir(SPIFFS, "/", 0);
}
