#include <stddef.h>

#ifndef OTA_H
#define OTA_H


class Ota
{   
public:
    void enableOTA(const char * password, const size_t port = 8266, const char *hostname = nullptr );
        void handleOTA();
};

#endif