#include <Arduino.h>

#ifndef OTA_H
#define OTA_H

class Ota
{
public:
    bool enabled = false;
    bool running = false;

    void enableOTA(const char *password, const uint16_t port, const char *hostname);
    void handleOTA();
};

#endif