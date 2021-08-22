#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

const char *net_radio = "net_radio";
const char *bluetooth = "bluetooth";
const char *airplay = "airplay";
const char *tuner = "tuner";
const char *spotify = "spotify";

// const char *music_cast_value[] = {net_radio, bluetooth, airplay, spotify, tuner};

class Yamaha
{
    String server;
    HTTPClient http;
    WiFiClient client;

public:
    Yamaha(const String &ipAddr) : server("http://" + ipAddr){};

    boolean powerOn()
    {
        http.begin(this->client, (this->server + "/YamahaExtendedControl/v1/main/setPower?power=on").c_str());
        return http.GET() == 200;
    };
    boolean powerOff()
    {
        http.begin(this->client, (this->server + "/YamahaExtendedControl/v1/main/setPower?power=standby").c_str());
        return http.GET() == 200;
    };

    boolean tooglePower()
    {
        http.begin(this->client, (this->server + "/YamahaExtendedControl/v1/main/setPower?power=toggle").c_str());
        return http.GET() == 200;
    };

    String checkPower()
    {
        http.begin(this->client, (this->server + "/YamahaExtendedControl/v1/main/getStatus").c_str());

        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, http.getStream());

        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return "error";
        }

        return doc["power"];
    }

    boolean setInput(const char *input)
    {
        http.begin(this->client, (this->server + "/YamahaExtendedControl/v1/main/setInput?input=" + input).c_str());
        return http.GET() == 200;
    }

    boolean setVolume(int value)
    {
        if (value < 0 || value > 100)
            return false;
        http.begin(this->client, (this->server + "/YamahaExtendedControl/v1/main/setVolume?volume=" + value).c_str());
        return http.GET() == 200;
    }
};