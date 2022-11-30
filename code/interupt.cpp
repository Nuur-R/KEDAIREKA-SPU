// Node01
// Gudang

#include <Arduino.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ACS712.h>

#define cpmPin D7
int count = 0;
bool bstat = false;
void ICACHE_RAM_ATTR impulseCount(void)
{
  bstat = true;
}
void setup()
{
  Serial.begin(115200);
  Serial.println("Setup");
  pinMode(cpmPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(cpmPin), impulseCount, CHANGE);
  Serial.println("End Setup");
}

void loop()
{
  if (bstat == true)
  {
    count++;
    Serial.print("Count: ");
    Serial.println(count);
    delay(500);
    bstat = false;
  }
}