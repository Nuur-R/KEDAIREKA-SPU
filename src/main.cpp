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


MAX6675 thermocouple_satu(thermoCSK_satu, thermoCS_satu, thermoSO_satu);

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void buzz(int pin, int delayTime, int repeat);
void sendData(float temperatur,
              float kelembapan,
              float arus,
              float biaya_listrik,
              int cycle,
              float daya,
              String lokasi,
              String nama_mesin,
              int pegawai_id,
              bool status_power,
              float tegangan,
              float total_produksi,
              float volume);
void powerDisplay(float arus, float daya);

String lokasi = "Gudang";
String node = "Node04";
String nama_mesin = "Mesin Auger";
String nodeName = node + "-" + nama_mesin;
String accessPointIP = "192.158.4.1";

int bahan_id = 1;

float temperatur_oven1 = 0;
float kelembapan = 0;

float tegangan = 220;
float arus = 0;
float daya = arus * tegangan;
float biaya_listrik = 0;
bool status_power = true;


int cycle = 1;
int pegawai_id = 1;

float total_produksi = 1230;
float volume = 100;
int bahan_id = 1;
float temperatur_oven1 = 0;


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
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Oven");
    display.setTextSize(1);
    display.setCursor(0,20);
    display.println("Temperatur : ");
    display.setCursor(0,30);
    display.println(temperatur_oven1);
    display.setCursor(0,40);
    display.display();
    delay(displayTime);

    if (millis() - lastTime > timerDelay)
    {
        lastTime = millis();
        sendData( temperatur_oven1,
                  kelembapan,
                  arus,
                  biaya_listrik,
                  cycle,
                  daya,
                  lokasi,
                  nama_mesin,
                  pegawai_id,
                  status_power,
                  tegangan,
                  total_produksi,
                  volume);
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


void sendData(float temperatur,
              float kelembapan,
              float arus,
              float biaya_listrik,
              int cycle,
              float daya,
              String lokasi,
              String nama_mesin,
              int pegawai_id,
              bool status_power,
              float tegangan,
              float total_produksi,
              float volume)
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
    doc["params"]["args"][3] = "kedaireka.mesin.monitoring";
    doc["params"]["args"][4] = "create";
    doc["params"]["args"][5][0]["temperatur"] = temperatur;
    doc["params"]["args"][5][0]["kelembapan"] = kelembapan;
    doc["params"]["args"][5][0]["arus"] = arus;
    doc["params"]["args"][5][0]["biaya_listrik"] = biaya_listrik;
    doc["params"]["args"][5][0]["cycle"] = cycle;
    doc["params"]["args"][5][0]["daya"] = daya;
    doc["params"]["args"][5][0]["lokasi"] = lokasi;
    doc["params"]["args"][5][0]["name"] = nama_mesin;
    doc["params"]["args"][5][0]["pegawai_id"] = pegawai_id;
    doc["params"]["args"][5][0]["status_power"] = status_power;
    doc["params"]["args"][5][0]["t_off"] = "2022-11-25 11:17:39";
    doc["params"]["args"][5][0]["t_on"] = "2022-11-25 10:35:39";
    doc["params"]["args"][5][0]["tegangan"] = tegangan;
    doc["params"]["args"][5][0]["total_produksi"] = total_produksi;
    doc["params"]["args"][5][0]["volume"] = volume;
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
