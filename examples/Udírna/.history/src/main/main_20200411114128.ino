#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341esp.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Ticker.h>
#include <Fonts/FreeMonoBoldOblique12pt7b.h>

#include "Plat.cpp"
#include <OneWire.h>
#include <DallasTemperature.h>

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

WiFiUDP ntpUDP;
// You can specify the time server pool and the offset, (in seconds)
// additionaly you can specify the update interval (in milliseconds).
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 1000 * 30);

#define ONE_WIRE_BUS D3

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// For the esp shield, these are the default.
#define TFT_DC 2
#define TFT_CS 15

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Ticker ticker;
Ticker ticker2;
Ticker tickerUp;
Ticker tickerPlatform;

const char *ssid = "Zahrada";
const char *password = "1414elektron151";

// const char * API_KEY = "818d9707745cb973d8f940987a449be0";	// Smejkal udirna
const char * API_KEY = "d7a8f5a98719835641193f760a6e1344";	// tata udirna


Plat plat(API_KEY);

void printWifiStatus(String, IPAddress);
void refreshTime();
void refreshTemperature();
void updateUpTime();
void sendTemperature();
void enableSend();
void configModeCallback(WiFiManager * myWifiManager);

void setup()
{
	Serial.begin(115200);
	delay(1000);
	// WiFi.disconnect();

	/* Display setup */
	SPI.setFrequency(ESP_SPI_FREQ);
	tft.begin();
	tft.setRotation(2);
	tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
	tft.fillScreen(ILI9341_BLACK);

	//tft.drawCircle(10, 246, 7, ILI9341_YELLOW);

	/* Show manual for wifi manager */
	tft.setCursor(0, 20);
	tft.setTextSize(2);
	tft.println("Pro nastaveni site:");
	tft.setTextSize(1);
	int offY = 41;
	tft.setCursor(0, offY);
	tft.println("1. pripojte se k wifi 'udirna'");
	tft.setCursor(0, offY + 10);
	tft.println("2. kliknete na prihlasit");
	tft.setCursor(0, offY + 10 *2);
	tft.println("   nebo otevrte http://192.168.4.1");
	tft.setCursor(0, offY + 10 *3);
	tft.println("3. vyberte 'Configure wifi'");
	tft.setCursor(0, offY + 10 *4);
	tft.println("4. zadejte ssid a heslo");

	tft.setCursor(0, 160);
	tft.println("Zapojeni sondy:");
	tft.fillRect(4, 173, 12, 40 + 4, ILI9341_DARKGREY);
	const int offY2 = 180;
	const int a = 6;
	tft.fillRect(7, offY2, a, a, ILI9341_BLACK);
	tft.fillRect(7, offY2 + 13, a, a, ILI9341_RED);
	tft.fillRect(7, offY2 + 13 * 2, a, a, ILI9341_YELLOW);

	tft.setCursor(0, 278);
	tft.println("IP adresa: 192.168.4.1");

	tft.setCursor(0, 290);
	tft.println("WIFI: connecting");

	/* Open captive portal */
	WiFiManager wifiManager;
	wifiManager.setAPCallback(configModeCallback);
	wifiManager.autoConnect("Udirna 🍖");

	/* Reset screen */
	tft.fillScreen(ILI9341_BLACK);

	/* Show wifi status */
	if (WiFi.status() != WL_CONNECTED) {
		printWifiStatus("WIFI: Can't connect to " + String(ssid), WiFi.localIP());
	} else {
		printWifiStatus("WIFI: connected to " + String(ssid), WiFi.localIP());
	}

	timeClient.begin();
	timeClient.setTimeOffset(3600 * 2);

	sensors.begin();

	tft.setFont(&FreeMonoBoldOblique12pt7b);
	ticker.attach(1, refreshTime);
	ticker2.attach_ms(2 * 1000, refreshTemperature);
  delay(100);
  tickerUp.attach(60, updateUpTime);
	tickerPlatform.attach(60, enableSend);

	tft.setCursor(0, 20);
	tft.setTextSize(1);
	tft.println("Teplota (*C):");
	tft.setCursor(0, 170);
	tft.println("Time:");

	tft.setFont();

	tft.setCursor(70, 303);
	tft.println("IOTplatforma.cloud");

	refreshTemperature();
	refreshTime();
	//sendTemperature();
}

void refreshTemperature();
void refreshTime();

bool SEND_ENABLED = true;

void loop()
{
	delay(100);
	if (SEND_ENABLED) {
		sendTemperature();
		SEND_ENABLED = false;
	}
}

int tempInvalid = 0;
double temperature = -127.;
void refreshTemperature()
{
	sensors.requestTemperatures();
	double temp = (int)(sensors.getTempCByIndex(0) * 100) / 100.;

	Serial.print("C = ");
	Serial.println(temp);

	//tft.fillRect(70, 50, 100, 70, ILI9341_BLACK);
	tft.setTextSize(8);
	tft.setCursor(0, 60);
	tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
	if (temp < -20)
	{
		if (tempInvalid == 3)
		{
			temperature = -127;
			tft.println("??.??");
			// tft.drawCircle(10, 246, 3, ILI9341_YELLOW);
		}
		else
		{
			++tempInvalid;
		}
	}
	else
	{	
		temperature = temp;
		tft.println(temp);
		tempInvalid = 0;
	}
	tft.setTextSize(1);
	tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
}

void printWifiStatus(String status, IPAddress ipAddr)
{
	tft.setCursor(0, 246);
	tft.println("Up time: 00:00 min");
	tft.setCursor(0, 258);
	tft.println("IP adresa: " + ipAddr.toString());

	tft.setCursor(0, 270);
	tft.println(status);
}

int UPhours = 0;
int UPmin = 0;

void updateUpTime()
{
	++UPmin;
	if (UPmin == 60)
	{
		UPmin = 0;
		++UPhours;
	}

	tft.setCursor(54, 246);
	String sHours = UPhours >= 10 ? UPhours : String("0") + UPhours;
	String sMin = UPmin >= 10 ? UPmin : String("0") + UPmin;
	tft.print(sHours + ":" + sMin);
}

void refreshTime()
{
	// timeClient.update();
	tft.setCursor(35, 190);
	tft.setTextSize(2);
	tft.println(timeClient.getFormattedTime());
	tft.setTextSize(1);
}

bool problem = false;
const int errorY = 283;
void sendTemperature() {
	if (temperature > -20) {
		int httpCode = plat.sendDataHTTPS(temperature);
		if (httpCode == 500) {
			tft.setCursor(0, errorY);
			tft.setTextColor(ILI9341_RED);
			tft.println("Platform problem detected!");
			tft.setTextColor(ILI9341_WHITE);
			problem=true;
		}else if (httpCode == -1) {
			tft.setCursor(0, errorY);
			tft.setTextColor(ILI9341_RED);
			tft.println("Can't connect to platform!");
			tft.setTextColor(ILI9341_WHITE);
			problem = true;
		}else if(problem){
			tft.fillRect(0, errorY, 240, 20, ILI9341_BLACK);
		}
	Serial.println(httpCode);
	} else {
		tft.setCursor(0, errorY);
		tft.setTextColor(ILI9341_RED);
		tft.println("Invalid temperature!");
		tft.setTextColor(ILI9341_WHITE);
		problem = true;
	}
}

void enableSend() {
	SEND_ENABLED = true;
}

void configModeCallback(WiFiManager *myWiFiManager) {
	Serial.println("callback");
	 Serial.println(WiFi.softAPIP());
  	//if you used auto generated SSID, print it
 	 Serial.println(myWiFiManager->getConfigPortalSSID());
	// printWifiStatus("Can't connect to " + String(ssid), WiFi.localIP());
}
