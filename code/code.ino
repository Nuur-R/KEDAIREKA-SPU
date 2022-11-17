#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ArduinoJson.h>

char ssid[] = "Family"; 
char password[] = "kf3092907";

const char* serverName = "http://18.217.56.118:8069/kedaireka/iot/add";

const char data[] = "{\"bahan_id\":\"1\",\"temperatur\":\"24.25\",\"kelembapan\":\"49.54\"}";
// Json data to send to the server

unsigned long lastTime = 0;
unsigned long timerDelay = 20000;

int bahan_id = 1;
float temperatur = 0;
float kelembapan = 0;

DynamicJsonDocument doc(1024);


void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}

void loop() {
  // random int for temperature and kelembaban
  temperatur = random(20, 37);
  kelembapan = random(40, 60);
  
  
  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
      
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverName);
  
      // If you need Node-RED/server authentication, insert user and password below
      //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
  
      doc["bahan_id"] = bahan_id;
      doc["temperatur"]   = temperatur;
      doc["kelembapan"] = kelembapan;

      serializeJson(doc, Serial);
      // If you need an HTTP request with a content type: application/json, use the following:
      http.addHeader("Content-Type", "application/json");
      int httpResponseCode = http.POST(doc.as<String>());

      // If you need an HTTP request with a content type: text/plain
      //http.addHeader("Content-Type", "text/plain");
      //int httpResponseCode = http.POST("Hello, World!");
     
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
        
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}
