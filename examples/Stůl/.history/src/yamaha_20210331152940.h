#include <ESP8266HTTPClient.h>

class Yamaha
{
    String server;
    HTTPClient http;

    Yamaha(const String &ipAddr) : server("http://" + ipAddr){};

    boolean tooglePower()
    {
        http.begin((this->server + "/YamahaExtendedControl/v1/main/setPower?power=toggle").c_str());
        return http.GET() == 200;
    };
};