// Oven Code

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>   

char ssid[] = "Family";
char password[] = "kf3092907";

const char *serverName = "http://18.217.56.118:8069/jsonrpc";

unsigned long lastTime = 0;
unsigned long timerDelay = 20000;

int bahan_id = 1;
float temperatur = 0;
float kelembapan = 0;

DynamicJsonDocument doc(1024);

void setup()
{
    Serial.begin(115200);
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
}

void loop()
{
    temperatur = random(150, 200);
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
            doc["params"]["args"][3] = "kedaireka.oven.monitoring";
            doc["params"]["args"][4] = "create";
            doc["params"]["args"][5][0]["oven_id"] = 2;
            doc["params"]["args"][5][0]["bahan_id"] = 1;
            doc["params"]["args"][5][0]["pegawai_id"] = 1;
            doc["params"]["args"][5][0]["volume"] = 5.0;
            doc["params"]["args"][5][0]["oven_on"] = "2022-11-17 05:10:40";
            doc["params"]["args"][5][0]["oven_off"] = "2022-11-17 05:15:40";
            doc["params"]["args"][5][0]["durasi_proses_oven"] = 5;
            doc["params"]["args"][5][0]["temperatur"] = temperatur;
            doc["params"]["args"][5][0]["kelembapan"] = kelembapan;
            doc["params"]["args"][5][0]["status_kualitas"] = "ok";
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
