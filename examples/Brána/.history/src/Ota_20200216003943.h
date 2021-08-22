#include <stddef.h>

#ifndef OTA_H
#define OTA_H


class Ota
{
public:
    bool enabled = false;

    void enableOTA(const char * password, const size_t port, const char *hostname );
    void handleOTA();
};

#endif