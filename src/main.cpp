#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <arial.h>
#include <SoftwareSerial.h>
#include "credentials.h"
#include <WifiController.h>
#include <OTAHandler.h>
#include "PubSubClient.h"
#include <ArduinoJson.h>
#include "ProductData.h"

WifiController wifiController = WifiController();
SoftwareSerial softwareSerial {};
WiFiClient wifiClient = WiFiClient();

PubSubClient mqttClient = PubSubClient();

const uint8_t displayCSPin = 32;
const uint8_t displayDCPin = 26;
const uint8_t displayRSTPin = 25;

Adafruit_ILI9341 display = Adafruit_ILI9341(displayCSPin, displayDCPin, displayRSTPin);

void maintainWifiConnectionRTOS(void* parameters)
{
    for (;;) {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        wifiController.maintainConnection();
    }
}

void maintainOTAConnection(void* parameters)
{
    OTAHandler::setEvents();
    OTAHandler::init();

    for (;;) {
        OTAHandler::maintainConnection();
    }
}

void messageHandler(char* topic, const byte* payload, unsigned int length)
{
    Serial.println("Got em boi");
    StaticJsonDocument<350> productDataAsJson;
    DeserializationError error = deserializeJson(productDataAsJson, payload);

    if (error) {
        display.setCursor(0, 20);
        display.setTextColor(ILI9341_GREEN);
        display.printf(
                "\nZkuste prosim znovu...\n\n");
        display.setTextColor(ILI9341_RED);
        display.printf("Admin -> Check if product is registered in db..."
                       "\nError message: %s", error.c_str());
        return;
    }

    ProductData productData = ProductData(
            productDataAsJson["Name"],
            productDataAsJson["Price"],
            productDataAsJson["Stock"],
            productDataAsJson["UnitOfMeasure"],
            productDataAsJson["UnitOfMeasureKoef"]);

    if (strcmp(productData.name, "") == 0) {
        display.fillScreen(ILI9341_BLACK);
        display.setCursor(0, 20);
        display.setTextSize(2);
        display.setTextColor(ILI9341_GREEN);
        display.printf("\nZkuste prosim\nznovu...\n");
        display.setTextSize(1);
        display.setTextColor(ILI9341_RED);
        display.printf("\nAdmin -> check if product exists in db...\n");
        return;
    }

    display.fillScreen(ILI9341_BLACK);
    display.setTextColor(ILI9341_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.printf("\n%s\n\n", productData.name);

    display.setTextColor(ILI9341_GREEN);
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

void setup()
{
    display.begin();
    display.setFont(&ariblk9pt8b);
    display.setRotation(1);
    display.fillScreen(ILI9341_BLACK);

    Serial.begin(115200);
    softwareSerial.begin(9600, SWSERIAL_8N1, 13, 15);

    wifiController.setHostname(hostname).setSSID(wiFiSSID).setPassword(wiFiPassword);
    wifiController.connect();

    xTaskCreatePinnedToCore(
            maintainWifiConnectionRTOS,
            "keepWifiAlive",
            5000,
            nullptr,
            1,
            nullptr,
            CONFIG_ARDUINO_RUNNING_CORE
    );

    xTaskCreatePinnedToCore(
            maintainOTAConnection,
            "keepOTAAlive",
            5000,
            nullptr,
            1,
            nullptr,
            CONFIG_ARDUINO_RUNNING_CORE
    );

    mqttClient.setServer(mqttServer, mqttPort);
    mqttClient.setClient(wifiClient);
    mqttClient.connect(hostname);
    mqttClient.subscribe(hostname);
    mqttClient.setCallback(messageHandler);
}

void loop()
{
    mqttClient.loop();

    if (softwareSerial.available() > 0) {
        uint8_t buffer[22];
        uint8_t size = softwareSerial.readBytesUntil(13, buffer, 22);
        char barcodeAsCharArray[size];
        memcpy(barcodeAsCharArray, buffer, size);

        char jsonBuffer[200];
        StaticJsonDocument<200> jsonDoc;
        jsonDoc["ClientTopic"] = hostname;
        jsonDoc["Barcode"] = barcodeAsCharArray;
        jsonDoc["IncludeDiacritics"] = false;
        serializeJson(jsonDoc, jsonBuffer);

        bool successfulPublish = mqttClient.publish(publishTopic, jsonBuffer, false);
        while (!successfulPublish) {
            mqttClient.disconnect();
            mqttClient.connect(hostname);
            mqttClient.subscribe(hostname);
            delay(100);
            successfulPublish = mqttClient.publish(publishTopic, jsonBuffer, false);
        }
    }
}
