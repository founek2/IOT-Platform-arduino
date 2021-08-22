#include <ESP8266HTTPClient.h>

enum MUSIC_CAST_INPUT
{
    NET_RADIO,
    BLUETOOTH,
    AIRPLAY,
    SPOTIFY,
    TUNER,
}

const **char music_cast_value = {"net_radio", "bluetooth", "airplay", "spotify", "tuner"};

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

    boolean setInput(MUSIC_CAST_INPUT input)
    {
        const res = await fetch(this.url + "/YamahaExtendedControl/v1/main/setInput?input=" + music_cast_value[input]);
        return res.status == = 200;
    }
};