#include <Arduino.h>
#include <WifiController.h>

WifiController wifiController{};

void setup()
{
    Serial.begin(115200);
    wifiController.setSSID("Gei").setPassword("1234567890");
    wifiController.setHostname("Shit");
}

void loop()
{

}