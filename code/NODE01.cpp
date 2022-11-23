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


const char *serverName = "http://18.217.56.118:8069/jsonrpc";

unsigned long lastTime = 0;
unsigned long timerDelay = 20000;

#define DHTPIN D4 
#define DHTTYPE DHT22
DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;

int bahan_id = 1;
float temperatur = 0;
float kelembapan = 0;

DynamicJsonDocument doc(1024);

void setup()
{
    Serial.begin(9600);
    Serial.println();
    WiFiManager wifiManager;
    wifiManager.setBreakAfterConfig(true);

    if (!wifiManager.autoConnect("SPU-Kedaireka")) {
        Serial.println("failed to connect, we should reset as see if it connects");
        delay(3000);
        ESP.reset();
        delay(5000);
    }

    Serial.println("connected...yeey :)");
    Serial.println("local ip");
    Serial.println(WiFi.localIP());
    Serial.println("Timer set to 20 seconds (timerDelay variable), it will take 20ö seconds before publishing the first reading.");

    dht.begin();
    Serial.println(F("DHTxx Unified Sensor Example"));
}

void loop()
{
    temperatur = random(20, 37);
    kelembapan = random(40, 60);

    if ((millis() - lastTime) > timerDelay)
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
            doc["params"]["args"][1] = 2;
            doc["params"]["args"][2] = "nimda0";
            doc["params"]["args"][3] = "kedaireka.iot";
            doc["params"]["args"][4] = "create";
            doc["params"]["args"][5][0]["temperatur"] = temperatur;
            doc["params"]["args"][5][0]["kelembapan"] = kelembapan;
            doc["id"] = 1;
            String json;
            serializeJson(doc, json);

            http.addHeader("Content-Type", "application/json");
            int httpResponseCode = http.POST(doc.as<String>());

            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);

            http.end();
        }
        else
        {
            Serial.println("WiFi Disconnected");
        }
        lastTime = millis();
    }
}
