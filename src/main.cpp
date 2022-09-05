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

struct ProductData
{
    const char* name {};
    float price {};
    const char* stock {};
    const char* unitOfMeasure {};
    float unitOfMeasureKoef {};

    ProductData(const char* name, float price, const char* stock, const char* unitOfMeasure,
                const float unitOfMeasureKoef)
    {
        this->name = name;
        this->price = price;
        this->stock = stock;
        this->unitOfMeasure = unitOfMeasure;
        this->unitOfMeasureKoef = unitOfMeasureKoef;
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
        uint8_t size = softwareSerial.readBytesUntil(13, barcodeBuffer, 22);
        char chars[size + 1];
        memcpy(chars, barcodeBuffer, size);
        chars[size] = '\0';

        char request_url[105];
        strcpy(request_url, requestURLWithoutBarcodeArgument);
        strcat(request_url, chars);

        Response response = getProductData(request_url);

        StaticJsonDocument<350> productDataAsJson;

        DeserializationError error = deserializeJson(productDataAsJson, response.payload.c_str());

        if (error) {
            display.setCursor(0, 20);
            display.setTextColor(ILI9341_RED);
            display.print(F("Unpacking json failed, try to scan again or product info is not registered...: "));
            display.println(error.f_str());
            return;
        }

        ProductData productData = ProductData(
                productDataAsJson["Name"],
                productDataAsJson["Price"],
                productDataAsJson["Stock"],
                productDataAsJson["UnitOfMeasure"],
                productDataAsJson["UnitOfMeasureKoef"]);

        display.setTextColor(ILI9341_WHITE);
        display.fillScreen(ILI9341_BLACK);
        display.setTextSize(1);
        display.setCursor(0, 20);
        display.println(productData.name);
        display.println();
        display.println();

        display.setTextColor(ILI9341_RED);
        display.setTextSize(2);
        display.printf("Cena: %.6g kc\n", productData.price);

        display.setTextColor(ILI9341_WHITE);

        if (strcmp(productData.unitOfMeasure, "") > 0) {
            display.setTextSize(1);
            display.printf("Cena za %s: %.6g kc\n\n",
                           productData.unitOfMeasure,
                           productData.price * productData.unitOfMeasureKoef);
        }

        display.setTextSize(1);
        display.printf("Stock: %s", productData.stock);
    }
}
