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
#include <ACS712.h>
#include <max6675.h>


const char *serverName = "http://18.217.56.118:8069/jsonrpc";

unsigned long lastTime = 0;
unsigned long timerDelay = 60000*15;
unsigned long displayTime = 2000;

#define BUZZPIN D0
int thermoSO = D6;
int thermoCS = D7;
int thermoCSK  = D8;


MAX6675 thermocouple(thermoCSK, thermoCS, thermoSO);

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
void startDisplay();
void wifiConnectedDisplay();
void ovenDisplay(String status, float temperature);
void powerDisplay(float arus, float daya);

String lokasi = "Gudang";
String node = "Node07";
String nama_mesin = "Mesin Oven 2";
String nodeName = node + "-" + nama_mesin;
String accessPointIP = "192.168.4.1";

int bahan_id = 1;

float suhu_min = 24;
float suhu_max = 37;
float temperatur_oven = 0;
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


DynamicJsonDocument doc(1024);
ACS712  ACS(A0, 5.0, 1023, 100);

void setup()
{
    Serial.begin(115200);
    WiFiManager wifiManager;
    wifiManager.setBreakAfterConfig(true);

    pinMode(BUZZPIN, OUTPUT);
    digitalWrite(BUZZPIN, LOW);

    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    
    startDisplay();

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
    wifiConnectedDisplay();
    buzz(BUZZPIN, 700, 3);
    Serial.println(WiFi.localIP());
    Serial.println("Timer set to 20 seconds (timerDelay variable), it will take 20 seconds before publishing the first reading.");

}

void loop()
{
  delay(100);
  float mA = ACS.mA_AC();
  Serial.println(mA);
  // arus mA ke ampare
  arus = mA / 1000;
  // menghitung daya listrik dari Mili Ampare dan tegangan
  daya = arus * tegangan;
  // menghitung biaya listrik dari daya listrik dan waktu
  biaya_listrik = daya * 0.000001 * 15;
  // menghitung total produksi dari volume dan cycle
  total_produksi = volume * cycle;
  // set status_power true jika arus lebih dari 0.23
  status_power = arus >= 0.05 ? true : false;
  
  temperatur_oven = thermocouple.readCelsius();
    ovenDisplay("Oven",temperatur_oven);
    delay(displayTime);
    powerDisplay(arus, daya);
    delay(displayTime);

    if (millis() - lastTime > timerDelay)
    {
        lastTime = millis();
        // sendData(1, temperatur_oven, 1);
        sendData(temperatur_oven,
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

// Display
void startDisplay(){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 10);
  display.println("Start...");
  display.setTextSize(1);
  display.setCursor(10, 30);
  display.println(nama_mesin);
  display.setCursor(10, 40);
  display.println(accessPointIP.c_str());
  display.display();
  buzz(BUZZPIN, 1500, 1);
  delay(500);
}
void wifiConnectedDisplay(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("WiFi Connected");
  display.setCursor(0, 10);
  display.println(WiFi.localIP());
  display.display();
}
void ovenDisplay(String status, float temperature){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(status);
  display.setTextSize(1);
  display.setCursor(0,25);
  display.println("Suhu Oven : ");
  display.setCursor(0,35);
  display.println(temperature);
  display.display();
}
void powerDisplay(float arus, float daya){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Power");
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println("Arus : ");
  display.setCursor(0, 30);
  display.println(arus);
  display.setCursor(0, 40);
  display.println("Daya : ");
  display.setCursor(0, 50);
  display.println(daya);
  display.display();
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
