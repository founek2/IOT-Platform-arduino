#include <ESP8266HTTPClient.h>


class Yamaha {
    String server;


    Yamaha(const String & ipAddr) : server("http://"+ipAddr ){};

    boolean tooglePower(){
        
    };
};