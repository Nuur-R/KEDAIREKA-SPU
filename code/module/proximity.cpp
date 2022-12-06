#include <Arduino.h>

#define PROXIMITY_PIN D7
void setup()
{
    Serial.begin(115200);
    pinMode(PROXIMITY_PIN, INPUT);
}

void loop()
{
    if (digitalRead(PROXIMITY_PIN) == HIGH)
    {
        Serial.println("Object Detected");
    }
    else
    {
        Serial.println("No Object Detected");
    }
}