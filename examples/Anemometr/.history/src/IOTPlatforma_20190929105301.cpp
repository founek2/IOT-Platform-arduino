#include <PubSubClient.h>
#include <WiFiClientSecure.h>

const char fingerprint[] = "7C 77 E9 A0 BB 42 C5 1D 09 1B E4 BF 7F 9E 32 A7 54 AD 4C 19";
const char *mqtt_server = "192.168.10.202";
// const char *mqtt_server = "test.iotplatforma.cloud";

WiFiClientSecure wifiClient;
PubSubClient client(wifiClient);

class IOTPlatforma{
    void init() {
        randomSeed(micros());
         wifiClient.setFingerprint(fingerprint);
             client.setServer(mqtt_server, 8883);
    }
};