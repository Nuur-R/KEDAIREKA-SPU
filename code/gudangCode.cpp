#include <Arduino.h>
/*
  Rui Santos
  Complete project details at Complete project details at https://RandomNerdTutorials.com/esp8266-nodemcu-http-get-post-arduino/

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  
  Code compatible with ESP8266 Boards Version 3.0.0 or above 
  (see in Tools > Boards > Boards Manager > ESP8266)
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ArduinoJson.h>

char ssid[] = "Family"; 
char password[] = "kf3092907";

const char* serverName = "http://18.217.56.118:8069/jsonrpc";

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
 
  Serial.println("Timer set to 20 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}

void loop() {
  temperatur = random(20, 37);
  kelembapan = random(40, 60);
  
  
  if ((millis() - lastTime) > timerDelay) {
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
      
      http.begin(client, serverName);
      
      // Blok JSON serialisation untuk struktur json berikut {"jsonrpc":"2.0","method":"call","params":{"service":"object","method":"execute_kw","args":["odoo_14_0",2,"nimda0","kedaireka.iot","create",[{"temperatur": 25,"kelembapan":40}]]},"id":1}
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
      
      // doc["bahan_id"] = bahan_id;
      // doc["temperatur"]   = temperatur;
      // doc["kelembapan"] = kelembapan;
      serializeJson(doc, Serial);
  
      http.addHeader("Content-Type", "application/json");
      int httpResponseCode = http.POST(doc.as<String>());

      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
        
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}
