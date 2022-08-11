#include <Arduino.h>
#include <WifiController.h>
#include "credentials.h"
#include <SoftwareSerial.h>

WifiController wifiController {};
SoftwareSerial softwareSerial {};

void maintainWifiConnectionRTOS(void* parameters)
{
    for (;;) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        wifiController.maintainConnection();
    }
}

void setup()
{
    Serial.begin(115200);
    softwareSerial.begin(9600, SWSERIAL_8N1, 19, 23);
    wifiController.setSSID(wifiSSID).setPassword(wifiPassword);
    wifiController.setHostname(hostname);

    xTaskCreatePinnedToCore(
            maintainWifiConnectionRTOS,
            "keepWifiAlive",
            5000,
            nullptr,
            1,
            nullptr,
            CONFIG_ARDUINO_RUNNING_CORE
    );
}

void loop()
{
    if (softwareSerial.available() > 0) {
        uint8_t dataBuffer[30];
        uint8_t size = softwareSerial.readBytesUntil(13, dataBuffer, 30);
        char chars[size+1];
        memcpy(chars, dataBuffer, size);
        chars[size] = '\0';
        Serial.println(chars);
    }
}
