#include <ESP8266HTTPClient.h>

enum MUSIC_CAST_INPUT
{
    NET_RADIO,
    BLUETOOTH,
    AIRPLAY,
    SPOTIFY,
    TUNER,
};

const char *net_radio = "net_radio";
const char *bluetooth = "bluetooth";
const char *airplay = "airplay";
const char *tuner = "tuner";
const char *spotify = "spotify";

const char *music_cast_value[] = {net_radio, bluetooth, airplay, spotify, tuner};

class Yamaha
{
    String server;
    HTTPClient http;

    Yamaha(const String &ipAddr) : server("http://" + ipAddr){};

    boolean powerOn()
    {
        http.begin((this->server + "/YamahaExtendedControl/v1/main/setPower?power=on").c_str());
        return http.GET() == 200;
    };
    boolean powerOff()
    {
        http.begin((this->server + "/YamahaExtendedControl/v1/main/setPower?power=standby").c_str());
        return http.GET() == 200;
    };

    boolean tooglePower()
    {
        http.begin((this->server + "/YamahaExtendedControl/v1/main/setPower?power=toggle").c_str());
        return http.GET() == 200;
    };

    boolean setInput(MUSIC_CAST_INPUT input)
    {
        http.begin((this->server + "/YamahaExtendedControl/v1/main/setInput?input=" + music_cast_value[input]).c_str());
        return http.GET() == 200;
    }

    boolean setVolume(uint16 value)
    {
        if (value > 100)
            return false;
        http.begin((this->server + "/YamahaExtendedControl/v1/main/setVolume?volume=" + value).c_str());
        return http.GET() == 200;
    }
};