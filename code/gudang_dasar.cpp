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

const char *serverName = "http://103.172.204.18:8069/jsonrpc";

unsigned long lastTime = 0;
unsigned long timerDelay = 60000 * 30;
unsigned long displayTime = 2000;

#define BUZZPIN D8
#define DHTPIN D4
#define DHTTYPE DHT22

#define cpmPin D7
volatile bool bstat = false;

DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin # (or -1 if sharing Arduino reset pin)
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
              float volume,
              int oven_id);
void startDisplay();
void wifiConnectedDisplay();
void mainDisplay(String status, float temperatur, float kelembapan, float arus, float daya, int cycle);

void ICACHE_RAM_ATTR impulseCount(void)
{
  bstat = true;
}

String lokasi = "Ruang Gudang";
String node = "Node01";
String nama_mesin = "Gudang";
String nodeName = node + "-" + nama_mesin;
String accessPointIP = "192.168.4.1";

int bahan_id = 1;

float suhu_min = 24;
float suhu_max = 37;
float temperatur = 0;
float kelembapan = 0;

float tegangan = 220;
float arus = 0;
float daya = arus * tegangan;
float biaya_listrik = 0;
bool status_power = true;

int cycle = 1;
int pegawai_id = 7;
int oven_id = 1;

float total_produksi = 1230;
float volume = 100;

DynamicJsonDocument doc(1024);
ACS712 ACS(A0, 5.0, 1023, 100);

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  WiFiManager wifiManager;
  wifiManager.setBreakAfterConfig(true);

  pinMode(BUZZPIN, OUTPUT);
  digitalWrite(BUZZPIN, LOW);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  startDisplay();

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

  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);

  Serial.println(__FILE__);
  Serial.print("ACS712_LIB_VERSION: ");
  Serial.println(ACS712_LIB_VERSION);

  ACS.autoMidPoint();
  Serial.print("MidPoint: ");
  Serial.print(ACS.getMidPoint());
  Serial.print(". Noise mV: ");
  Serial.println(ACS.getNoisemV());
  delayMS = sensor.min_delay / 1000;

  // set LIMIT_SWITCH as input
  pinMode(cpmPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(cpmPin), impulseCount, CHANGE);
}

void loop()
{
  delay(delayMS);
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature))
  {
    Serial.println(F("Error reading temperature!"));
  }
  else
  {
    temperatur = event.temperature;
  }
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity))
  {
    Serial.println(F("Error reading humidity!"));
  }
  else
  {
    kelembapan = event.relative_humidity;
  }

  float mA = ACS.mA_AC();
  Serial.println(mA);
  if (bstat == true)
  {
    cycle++;
    delay(300);
    bstat = false;
  }

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

  if (temperatur <= suhu_min)
  {
    mainDisplay("Dingin", temperatur, kelembapan, arus, daya, cycle);
    buzz(BUZZPIN, 1000, 3);
  }
  else if (temperatur >= suhu_max)
  {
    mainDisplay("Panas", temperatur, kelembapan, arus, daya, cycle);
    buzz(BUZZPIN, 300, 5);
  }
  else
  {
    mainDisplay("Normal", temperatur, kelembapan, arus, daya, cycle);
  }

  if (millis() - lastTime > timerDelay)
  {
    lastTime = millis();
    Serial.println("Publishing sensor values...");
    sendData(temperatur,
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
             volume,
             oven_id);
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
void startDisplay()
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 10);
  display.println("Start...");
  display.setTextSize(1);
  display.setCursor(10, 30);
  display.println(nodeName.c_str());
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
void mainDisplay(String status, float temperatur, float kelembapan, float arus, float daya, int cycle)
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(status);
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println("Suhu       : " + String(temperatur) + " C");
  display.println("Kelembapan : " + String(kelembapan) + " %");
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
              float volume,
              int oven_id)
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
    doc["params"]["args"][0] = "new_spu";
    doc["params"]["args"][1] = 44;
    doc["params"]["args"][2] = "12341234";
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
    String payload = http.getString();
    Serial.println("Response: ");
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
  lastTime = millis();
}