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
#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>

const char *serverName = "http://103.172.204.18:8069/jsonrpc";

unsigned long lastTime = 0;
unsigned long timerDelay = 10000;
unsigned long displayTime = 2000;

#define BUZZPIN D4
int thermoSO = D6;
int thermoCS = D5;
int thermoCSK = D0;

MAX6675 thermocouple(thermoCSK, thermoCS, thermoSO);

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


String lokasi = "Ruang Tester";
String node = "NodeXX";
String nama_mesin = "Mesin Test";
String nodeName = node + "-" + nama_mesin;
String accessPointIP = "192.168.4.1";

int bahan_id = 1;
int oven_id = 4;

float suhu_min = 24;
float suhu_max = 37;
float temperatur_oven = 0;
float kelembapan = 0;

float tegangan = 0;
float arus = 0;
float daya = arus * tegangan;
float biaya_listrik = 0;
bool status_power = true;

String t_on = "2020-01-01 00:00:00";
String t_off = "2020-01-01 00:00:00";

int cycle = 1;
int pegawai_id = 7;

float total_produksi = 1230;
float volume = 100;

DynamicJsonDocument doc(1024);

#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN D8
#define PZEM_TX_PIN D7
#endif

SoftwareSerial pzemSWSerial(PZEM_RX_PIN, PZEM_TX_PIN);
PZEM004Tv30 pzem(pzemSWSerial);


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
void startDisplay()
{
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
void wifiConnectedDisplay()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("WiFi Connected");
  display.setCursor(0, 10);
  display.println(WiFi.localIP());
  display.display();
}
void mainDisplay(String status, float temperatur, float tegangan, float arus, float daya)
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(status);
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println("Suhu     : " + String(temperatur) + " C");
  display.setCursor(0, 30);
  display.println("Tegangan : " + String(tegangan) + " V");
  display.setCursor(0, 40);
  display.println("Arus     : " + String(arus) + " A");
  display.setCursor(0, 50);
  display.println("Daya     : " + String(daya) + " W");
  display.display();
}

void sendData(float temperatur,
              float kelembapan,
              float arus,
              float biaya_listrik,
              int cycle,
              float daya,
              String lokasi,
              int oven_id,
              int pegawai_id,
              bool status_power,
              String t_off,
              String t_on,
              float tegangan,
              float total_produksi,
              float volume)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    WiFiClient client;

    http.begin(client, serverName);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST("{\"jsonrpc\": \"2.0\",\"method\": \"call\",\"params\": {\"service\": \"object\",\"method\": \"execute_kw\",\"args\": [\"new_spu\",44,\"12341234\",\"kedaireka.mesin.monitoring\",\"create\",[{\"temperatur\": " + String(temperatur_oven) + ",\"kelembapan\": " + String(kelembapan) + ",\"arus\": " + String(arus) + ",\"biaya_listrik\": " + String(biaya_listrik) + ",\"cycle\": " + String(cycle) + ",\"daya\": " + String(daya) + ",\"lokasi\": \"kedai reka\",\"oven_id\": " + String(oven_id) + ",\"pegawai_id\": " + String(pegawai_id) + ",\"status_power\": " + String(status_power) + ",\"t_off\": \"2022-11-25 11:17:39\",\"t_on\": \"2022-11-25 10:35:39\",\"tegangan\": " + String(tegangan) + ",\"total_produksi\": " + String(total_produksi) + ",\"volume\": " + String(volume) + "}]]},\"id\": 1}");
    Serial.println(httpCode);
    String payload = http.getString();
    Serial.println(payload);

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
}

// =    =    =   =  =  =
// = = ==   ===  =  == =
// =  = =  =  =  =  = ==

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

  if (!wifiManager.autoConnect(nodeName.c_str()))
  {
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

  temperatur_oven = thermocouple.readCelsius();
  float tegangaan = pzem.voltage();
  float arus = pzem.current();
  float daya = pzem.power();
  float biaya_listrik = pzem.energy();
  bool status_power = arus > 0.1 ? true : false;
  mainDisplay("Oven 1", temperatur_oven, tegangaan, arus, daya);

  if (millis() - lastTime > timerDelay)
  {
    lastTime = millis();
    sendData(temperatur_oven,
             kelembapan,
             arus,
             biaya_listrik,
             cycle,
             daya,
             lokasi,
             oven_id,
             pegawai_id,
             status_power,
             t_off,
             t_on,
             tegangaan,
             total_produksi,
             volume);
    
  }
}

