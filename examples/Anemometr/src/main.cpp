#include "myconfig.h"
#include <IOTPlatform.h>
#include <Queue.h>
#include <Ticker.h>

IOTPlatform plat("Anemometr");

unsigned long next_timestamp = 0;
volatile unsigned long i = 0;
float wind = 0;
float last_wind = 0;
int count = 0;
volatile unsigned long last_micros;
long debouncing_time = 5; // in millis
int input_pin = 13;
char charBuffer[32];

Ticker tickerSend;

void IRAM_ATTR Interrupt()
{
  if ((long)(micros() - last_micros) >= debouncing_time * 1000)
  {
    i++;
    last_micros = micros();
    // Serial.println("otocka");
  }
}

const int sec_window = 15;
bool SEND_MEASUREMENTS = false;
Property *propSpeed;
void setup()
{
  // plat.reset();
  Serial.begin(115200);
  delay(10);
  pinMode(input_pin, INPUT_PULLUP); // D7
  pinMode(A0, INPUT);

  Node *node = plat.NewNode("anemometr", "Vítr", NodeType::SENSOR);
  propSpeed = node->NewProperty("wind", "Rychlost", DataType::FLOAT);
  propSpeed->setClass(PropertyClass::PRESSURE);
  propSpeed->setUnit("km/h");

  plat.enableOTA("123456777", 8266, "esp-anemo");

  attachInterrupt(input_pin, Interrupt, RISING);

  tickerSend.attach(5 * 60, []()
                    { SEND_MEASUREMENTS = true; });
}

const int q_size = 10;
Queue<float, q_size> queue;

void addMeasurement(float &value)
{
  if (queue.size() >= q_size)
  {
    queue.dequeue();
  }
  queue.enqueue(value);
}

float calcMeasurementMean()
{
  double tmp = 0;
  for (int i = 0; i < queue.size(); i++)
  {
    tmp += queue.at(i);
  }

  return tmp / queue.size();
}

void loop()
{
  // Serial.print("rpm: ");
  // Serial.println(r);
  // delay(40);

  if (millis() > next_timestamp)
  {
    detachInterrupt(input_pin);
    count++;
    float rps = (i / number_reed) / sec_window; // computing rounds per second - every 3s
    if (i == 0)
      wind = 0.0;
    else
      wind = 1.761 / (1 + rps) + 3.013 * rps; // found here: https://www.amazon.de/gp/customer-reviews/R3C68WVOLJ7ZTO/ref=cm_cr_getr_d_rvw_ttl?ie=UTF8&ASIN=B0018LBFG8 (in German)

    if (last_wind - wind > 0.8 || last_wind - wind < -0.8 || count >= 10)
    {
      if (debugOutput)
      {
        Serial.print("Wind: ");
        Serial.print(wind);
        Serial.println(" km/h");
        Serial.print(wind / 3.6);
        Serial.println(" m/s");
      }

      addMeasurement(wind);

      count = 0;
    }
    i = 0;
    last_wind = wind;
    next_timestamp = millis() + sec_window * 1000; // sec into milices
    attachInterrupt(input_pin, Interrupt, RISING);
  }

  yield();
  plat.loop();

  if (SEND_MEASUREMENTS)
  {
    SEND_MEASUREMENTS = false;
    float speed = calcMeasurementMean();
    propSpeed->setValue(String(speed).c_str());

    while (!queue.isEmpty())
    {
      queue.dequeue();
    }
  }
}