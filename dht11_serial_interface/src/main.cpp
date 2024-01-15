#include <Arduino.h>
#include "DHT.h"
#include "SoftwareSerial.h"
#include "Adafruit_Sensor.h"

#define DHT_PIN     2
#define DHT_TYPE    DHT11

typedef struct {
    uint8_t temperature;
    uint8_t humidity;
    uint8_t heatIndex;
} DHTData;

DHT dht(DHT_PIN, DHT_TYPE);
SoftwareSerial interface(10, 11); // RX, TX

void setup() {  
    interface.begin(115200);
    dht.begin();
}

void loop() {
    DHTData data;

    data.temperature = dht.readTemperature();
    data.humidity = dht.readHumidity();

    if (!isnan(data.temperature) && !isnan(data.humidity)) 
    {
        data.heatIndex = dht.computeHeatIndex(data.temperature, data.humidity, false);

        for (size_t i = 0; i < sizeof(DHTData); i++) 
        {
          interface.print(((byte*)&data)[i], HEX);
          interface.print(" ");
        }

        interface.println();
    }

    delay(250);
}