#include <Arduino.h>
#include <WifiController.h>
#include <Adafruit_ILI9341.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Fonts/arial.h>
#include "credentials.h"
#include <HTTPClient.h>
#include <utility>
#include <ArduinoJson.h>

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

struct Response
{
    String payload {};
    int code {};

    Response(int code, String payload)
    {
        this->code = code;
        this->payload = std::move(payload);
    }
};

Response getProductData(const char* requestUrl)
{
    WiFiClient client;
    HTTPClient http;

    http.begin(client, requestUrl);

    Response response = Response(http.GET(), http.getString());

    http.end();

    return response;
}

void setup()
{
    display.begin();
    display.setFont(&ariblk9pt8b);
    display.setRotation(3);
    display.fillScreen(ILI9341_BLACK);

    Serial.begin(115200);
    softwareSerial.begin(9600, SWSERIAL_8N1, 13, 15);

    wifiController.setHostname(hostname).setSSID(wiFiSSID).setPassword(wiFiPassword);
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
        uint8_t barcodeBuffer[22];
        uint8_t size = softwareSerial.readBytesUntil(13, barcodeBuffer, 30);
        char chars[size + 1];
        memcpy(chars, barcodeBuffer, size);
        chars[size] = '\0';

        /*const char* requestArguments = "?barcode=8594052880977&include_diacritics=false";
        char* request_url = new char[strlen(serverName) + strlen(requestArguments) + 1];
        strcpy(request_url, serverName);
        strcat(request_url, requestArguments);*/

        char request_url [50];
        strcat

        Response response = getProductData(request_url);
        Serial.println(response.payload);

        StaticJsonDocument<250> productData;

        DeserializationError error = deserializeJson(productData, response.payload.c_str());

        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
        }

        const char* name = productData["Name"];

        Serial.println(name);

        /*display.setTextColor(ILI9341_WHITE);
        display.fillScreen(ILI9341_BLACK);
        display.setTextSize(1);
        display.setCursor(0, 20);
        display.println("Bylinna mast pro psy 50g");
        display.println();
        display.println();

        display.setTextColor(ILI9341_RED);
        display.setTextSize(2);
        display.println("Cena: 50 kc");*/
    }
}
