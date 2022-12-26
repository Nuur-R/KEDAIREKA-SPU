// author: @nuur_r
// version: 2.0.0
// changelog: update sensor arus dan tegangan  PZEM004T

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
DynamicJsonDocument doc(1024);

// Time Setting
unsigned long lastTime    = 0;
unsigned long timerDelay  = 60000*30;  // delay waktu pengiriman data
unsigned long displayTime = 2000;


#define DHTTYPE DHT22
// PIN Declaration
#define BUZZPIN D0
#define DHTPIN D4
#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN D8
#define PZEM_TX_PIN D7
#endif

// MAX6675 Declaration
DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;
// PZEM Declaration
SoftwareSerial pzemSWSerial(PZEM_RX_PIN, PZEM_TX_PIN);
PZEM004Tv30 pzem(pzemSWSerial);
// OLED Declaration
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Node Name
String lokasi         = "Ruang Packaging";
String node           = "Node05";
String nama_mesin     = "Mesin Vertikal";
String nodeName       = node + "-" + nama_mesin;
String accessPointIP  = "192.168.4.1";

// get data
float suhu_min  = 25;
float suhu_max  = 30;

int bahan_id    = 1;
int oven_id     = 3;
int pegawai_id  = 7;

// Post Data
float temperatur = 0;
float kelembapan      = 0;
float arus            = 0;
float tegangan        = 0;
float biaya_listrik   = 0;
float daya            = arus * tegangan;
int cycle             = 0;
float total_produksi  = 0;
float volume          = 0;
bool status_power     = true;
String t_on           = "2020-01-01 00:00:00";
String t_off          = "2020-01-01 00:00:00";





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

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
//  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = 
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

// Display Function
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
void mainDisplay(float temperatur, float kelembapan, float tegangan, float arus, float daya)
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.println("Suhu       : " + String(temperatur) + " C");
  display.setCursor(0, 20);
  display.println("Kelembapan : " + String(kelembapan) + " %");
  display.setCursor(0, 30);
  display.println("Tegangan   : " + String(tegangan) + " V");
  display.setCursor(0, 40);
  display.println("Arus       : " + String(arus) + " A");
  display.setCursor(0, 50);
  display.println("Daya       : " + String(daya) + " W");
  display.display();
}
void alertDisplay()
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Alert!!!");
  display.setCursor(0, 10);
  display.println(WiFi.localIP());
  display.display();
}

// Send Data Function
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
    int httpCode = http.POST("{\"jsonrpc\": \"2.0\",\"method\": \"call\",\"params\": {\"service\": \"object\",\"method\": \"execute_kw\",\"args\": [\"new_spu\",44,\"12341234\",\"kedaireka.mesin.monitoring\",\"create\",[{\"temperatur\": " + String(temperatur) + ",\"kelembapan\": " + String(kelembapan) + ",\"arus\": " + String(arus) + ",\"biaya_listrik\": " + String(biaya_listrik) + ",\"cycle\": " + String(cycle) + ",\"daya\": " + String(daya) + ",\"lokasi\": " + String(lokasi) + ",\"oven_id\": " + String(oven_id) + ",\"pegawai_id\": " + String(pegawai_id) + ",\"status_power\": " + String(status_power) + ",\"t_off\": " + String(t_off) + ",\"t_on\": " + String(t_on) + ",\"tegangan\": " + String(tegangan) + ",\"total_produksi\": " + String(total_produksi) + ",\"volume\": " + String(volume) + "}]]},\"id\": 1}");
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
// Fetch Data Function
void fetchData(){
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
    doc["params"]["args"][3] = "ir.config_parameter";
    doc["params"]["args"][4] = "search_read";
    doc["params"]["args"][5][0][0][0] = "key";
    doc["params"]["args"][5][0][0][1] = "like";
    doc["params"]["args"][5][0][0][2] = "kedaireka";
    doc["params"]["args"][5][1][0] = "key";
    doc["params"]["args"][5][1][1] = "value";
    doc["id"] = 1;

    String json;
    serializeJson(doc, json);

    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(doc.as<String>());

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println("Response: ");
    // Serial.println(payload);
    // {"jsonrpc": "2.0","id": 1,"result": [{"id": 62,"key": "kedaireka.kelas_pln","value": "1"},{"id": 55,"key": "kedaireka.rh_max","value": "80"},{"id": 54,"key": "kedaireka.rh_min","value": "60"},{"id": 59,"key": "kedaireka.rh_ruang_max","value": "80"},{"id": 58,"key": "kedaireka.rh_ruang_min","value": "60"},{"id": 57,"key": "kedaireka.suhu_ruang_max","value": "30"},{"id": 56,"key": "kedaireka.suhu_ruang_min","value": "25.0"},{"id": 63,"key": "kedaireka.tdl","value": "1300.0"},{"id": 53,"key": "kedaireka.temperatur_max","value": "30"},{"id": 52,"key": "kedaireka.temperatur_min","value": "25.0"},{"id": 61,"key": "kedaireka.waktu_oven_max","value": "360"},{"id": 60,"key": "kedaireka.waktu_oven_min","value": "30"}]}
    // mengambil data dari array json
    // Stream& input;

    DynamicJsonDocument doc(1536);

    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    const char* jsonrpc = doc["jsonrpc"]; // "2.0"
    int id = doc["id"]; // 1

    for (JsonObject result_item : doc["result"].as<JsonArray>()) {
      int result_item_id = result_item["id"]; // 62, 55, 54, 59, 58, 57, 56, 63, 53, 52, 61, 60
      const char* result_item_key = result_item["key"]; // "kedaireka.kelas_pln", "kedaireka.rh_max", ...
      const char* result_item_value = result_item["value"]; // "1", "80", "60", "80", "60", "30", "25.0", ...
    }
    suhu_min = doc["result"][10]["value"];
    suhu_max = doc["result"][9]["value"];
  }
}
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
//  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = 
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

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

  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);

  Serial.println(WiFi.localIP());
  Serial.println("Timer set to 20 seconds (timerDelay variable), it will take 20 seconds before publishing the first reading.");
}

void loop()
{
  delay(100);

  // update data
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
  tegangan      = pzem.voltage();
  arus          = pzem.current();
  daya          = pzem.power();
  biaya_listrik = pzem.energy();
  status_power  = arus > 0.1 ? true : false;

  if (temperatur < suhu_max || temperatur > suhu_min)
  {
    mainDisplay(temperatur, kelembapan, tegangan, arus, daya);
  }
  else
  {
    alertDisplay();
    buzz(BUZZPIN, 700, 3);
  }

  if (millis() - lastTime > timerDelay)
  {
    lastTime = millis();
    sendData(temperatur,
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
             tegangan,
             total_produksi,
             volume);
    
  }
}

