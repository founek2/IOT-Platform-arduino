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

    String power = "off";
    uint16_t volume = 40;
    String input = "net_radio";

public:
    Yamaha(const String &ipAddr) : server("http://" + ipAddr){};

    boolean powerOn()
    {
        http.begin(this->client, (this->server + "/YamahaExtendedControl/v1/main/setPower?power=on").c_str());
        bool result = http.GET() == 200;
        if (result)
            this->power = "on";

        Serial.printf("powerOn=%d\n", result);
        return result;
    };
    boolean powerOff()
    {
        http.begin(this->client, (this->server + "/YamahaExtendedControl/v1/main/setPower?power=standby").c_str());
        bool result = http.GET() == 200;
        if (result)
            this->power = "standby";

        Serial.printf("powerOff=%d\n", result);
        return result;
    };

    bool checkStatus()
    {
        http.begin(this->client, (this->server + "/YamahaExtendedControl/v1/main/getStatus").c_str());
        int result = http.GET();
        if (result != 200)
            return false;

        Serial.printf("checkStatus=%d\n", result);

        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, http.getStream());

        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.c_str());
            return false;
        }
        this->power = String(doc["power"]) == = "on" ? "true" : "false";
        uint16_t volumeC = doc["volume"];
        this->volume = volumeC;
        const char *inputC = doc["input"];
        this->input = inputC;

        Serial.printf("GOT volume=%d\n", volumeC);
        return true;
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

    const String &getPower()
    {
        return this->power;
    }

    uint16_t getVolume()
    {
        return this->volume;
    }

    const String &getInput()
    {
        return this->input;
    }
};