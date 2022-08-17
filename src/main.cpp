#include <Arduino.h>
#include <WifiController.h>
#include "credentials.h"
#include <Adafruit_ILI9341.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Fonts/FreeSansBold9pt7b.h>

WifiController wifiController {};
SoftwareSerial softwareSerial {};

const uint8_t displayCSPin = 32;
const uint8_t displayDCPin = 26;
const uint8_t displayRSTPin = 25;

Adafruit_ILI9341 display = Adafruit_ILI9341(displayCSPin, displayDCPin, displayRSTPin);

void maintainWifiConnectionRTOS(void* parameters)
{
    for (;;) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        wifiController.maintainConnection();
    }
}

void setup()
{
    display.begin();
    display.setRotation(3);
    display.fillScreen(ILI9341_BLACK);
    display.setCursor(0, 0);
    display.setTextColor(ILI9341_WHITE);
    display.setTextSize(3);
    display.println("Nazev: Prirodni mydlo 50g");
    display.println();
    display.setTextSize(4);
    display.setTextColor(ILI9341_MAGENTA);
    display.println("Cena: 50 kc");
    display.println();
    display.setTextSize(2);
    display.setTextColor(ILI9341_WHITE);
    display.println("Stock: 2");
    display.println();
    Serial.begin(115200);
    /*softwareSerial.begin(9600, SWSERIAL_8N1, 13, 15);
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
    );*/
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
