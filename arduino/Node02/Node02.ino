// Node02
// Oven

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
#include "max6675.h"


const char *serverName = "http://18.217.56.118:8069/jsonrpc";

unsigned long lastTime = 0;
unsigned long timerDelay = 20000;
unsigned long displayTime = 2000;
String nodeName = "Node01-Gudang";
String accessPointIP = "192.158.4.1";

#define BUZZPIN D0
int thermoSO_satu  = D6;
int thermoCS_satu = D7;
int thermoCSK_satu  = D8;

int thermoSO_dua  = D3;
int thermoCS_dua = D4;
int thermoCSK_dua  = D5;

MAX6675 thermocouple_satu(thermoCSK_satu, thermoCS_satu, thermoSO_satu);
MAX6675 thermocouple_dua(thermoCSK_dua, thermoCS_dua, thermoSO_dua);

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void buzz(int pin, int delayTime, int repeat);
void sendData(int id, float suhu, float humidity);

int bahan_id = 1;
float temperatur_oven1 = 0;
float temperatur_oven2 = 0;


DynamicJsonDocument doc(1024);

void setup()
{
    Serial.begin(115200);
    WiFiManager wifiManager;
    wifiManager.setBreakAfterConfig(true);

    pinMode(BUZZPIN, OUTPUT);
    digitalWrite(BUZZPIN, LOW);

    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10,10);
    display.println("Start...");
    display.setTextSize(1);
    display.setCursor(10,30);
    display.println(nodeName.c_str());
    display.setCursor(10,40);
    display.println(accessPointIP.c_str());
    display.display();
    buzz(BUZZPIN, 1500, 1);
    delay(500);
    

    if (!wifiManager.autoConnect(nodeName.c_str())) {
        Serial.println("failed to connect, we should reset as see if it connects");
        delay(3000);
        ESP.reset();
        delay(5000);
    }

    Serial.println("connected...yeey :)");
    Serial.println("local ip");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("WiFi Connected");
    display.setCursor(0,10);
    display.println(WiFi.localIP());
    display.display();
    buzz(BUZZPIN, 700, 3);
    Serial.println(WiFi.localIP());
    Serial.println("Timer set to 20 seconds (timerDelay variable), it will take 20 seconds before publishing the first reading.");

}

void loop()
{

  temperatur_oven1 = thermocouple_satu.readCelsius();
  temperatur_oven2 = thermocouple_dua.readCelsius();
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Dingin...");
    display.setTextSize(1);
    display.setCursor(0,20);
    display.println("oven 1 : ");
    display.setCursor(0,30);
    display.println(temperatur_oven1);
    display.setCursor(0,40);
    display.println("oven 2 : ");
    display.setCursor(0,50);
    display.println(temperatur_oven2);
    display.display();
    delay(displayTime);

    if (millis() - lastTime > timerDelay)
    {
        lastTime = millis();
        sendData(1, temperatur_oven1, 1);
    }
}

void buzz(int pin, int delayTime, int repeat)
{
    for (int i = 0; i < repeat; i++)
    {
        digitalWrite(pin, HIGH);
        delay(delayTime);
        digitalWrite(pin, LOW);
        delay(delayTime);
    }
}

void sendData(int id, float suhu, float humidity)
{
  if (WiFi.status() == WL_CONNECTED)  
  {
    WiFiClient client;
    HTTPClient http;

    http.begin(client, serverName);

    doc["jsonrpc"] = "2.0";
    doc["method"] = "call";
    doc["params"]["service"] = "object";
    doc["params"]["method"] = "execute_kw";
    doc["params"]["args"][0] = "odoo_14_0";
    doc["params"]["args"][1] = id;
    doc["params"]["args"][2] = "nimda0";
    doc["params"]["args"][3] = "kedaireka.iot";
    doc["params"]["args"][4] = "create";
    doc["params"]["args"][5][0]["temperatur"] = suhu;
    doc["params"]["args"][5][0]["kelembapan"] = humidity;
    doc["id"] = 1;
    String json;
    serializeJson(doc, json);

    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(doc.as<String>());

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("Send Data");
    display.display();
    delay(displayTime);
    http.end();
  }
  else
  {
    Serial.println("WiFi Disconnected");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("WiFi Disconnected");
    display.display();
    delay(displayTime);
  }
  lastTime = millis();
}