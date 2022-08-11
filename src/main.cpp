#include <Arduino.h>
#include <WifiController.h>
#include "credentials.h"

WifiController wifiController{};

void setup()
{
    Serial.begin(115200);
    wifiController.setSSID(wifiSSID).setPassword(wifiPassword);
    wifiController.setHostname("Shit");
}

void loop()
{
    wifiController.maintainConnection();
}
