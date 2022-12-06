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

bool bstat = false;
void impulseCount(void)
{
  bstat = true;
}
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup()
{
  Serial.begin(115200);
  Serial.println("Setup");
  pinMode(cpmPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(cpmPin), impulseCount, CHANGE);
  Serial.println("End Setup");
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Setup");
  display.display();
  delay(1500);
}

int count = 0;
void loop()
{
  if (bstat == true)
  {
    count++;
    Serial.print("Count: ");
    Serial.println(count);
    delay(200);
    bstat = false;
  }
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Count: ");
  display.println(count);
  display.display();
  
}