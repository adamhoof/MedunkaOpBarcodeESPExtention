#include <Arduino.h>
#include <WifiController.h>
#include <Adafruit_ILI9341.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Fonts/FreeMonoBoi.h>

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
    display.setFont(&FreeMono9pt8b);
    display.setRotation(3);
    display.fillScreen(ILI9341_BLACK);
    display.setCursor(0, 0);
    display.setTextColor(ILI9341_WHITE);
    display.setTextSize(1);
    display.println();
    display.println("ěščřžýáíé");

    Serial.begin(115200);
    softwareSerial.begin(9600, SWSERIAL_8N1, 13, 15);

    /*wifiController.setHostname("MedunkaOPBarcodeUpdate").setSSID("Gei").setPassword("1234567890");
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
