# IoT Platform client

This library provides complex Arduino client for communication with [IoT Platform](https://prod.iotplatforma.cloud). You can specify all nodes and properties (including callback) and library will automatically handle announcing and pairing with platform. Refer to [examples](./examples/) to see how to use it.

## Compatible Hardware

The library uses the PubSubClient for interacting with the
MQTT. Currently, these boards are supported:

- ESP8266 - mqtt library used is [PubSubClient](https://github.com/knolleary/pubsubclient)
- ESP32 - mqtt library used is [arduino-mqtt](https://github.com/256dpi/arduino-mqtt)

> arduino-mqtt might work even with esp8266, but I did not test it. I have good experience with PubSubClient

## License

This code is released under the MIT License.
