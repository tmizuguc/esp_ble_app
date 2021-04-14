#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "ble.h"

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
extern bool deviceConnected = false;
extern bool oldDeviceConnected = false;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID "63b803e2-9201-47ee-968b-1405602a1b8e"
#define CHARACTERISTIC_UUID "46bfca8b-b8d8-40b1-87e7-c22116324c01"

#define MD1_OUT1 32 //GPIO #32
#define MD1_OUT2 33 //GPIO #33
#define MD2_OUT1 25 //GPIO #25
#define MD2_OUT2 26 //GPIO #26

char buf[100];

class MyServerCallbacks : public BLEServerCallbacks
{
private:
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
        BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
    }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
private:
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string value = pCharacteristic->getValue();

        if (value.length() > 0)
        {

            Serial.println("**********");

            if (value == "State: 0")
            {
                sprintf(buf, "GPIO[%d,%d] out1 = 0, out2 = 0;", MD1_OUT1, MD1_OUT2);
                Serial.println(buf);
                sprintf(buf, "P %3.2f: G %3.2f", 0.0, 0.0);
                Serial.println(buf);
                digitalWrite(MD1_OUT1, LOW);
                digitalWrite(MD1_OUT2, LOW);
                digitalWrite(MD2_OUT1, LOW);
                digitalWrite(MD2_OUT2, LOW);
            }
            else if (value == "State: 1")
            {
                sprintf(buf, "GPIO[%d,%d] out1 = 1, out2 = 0;", MD1_OUT1, MD1_OUT2);
                Serial.println(buf);
                sprintf(buf, "P %3.2f: G %3.2f", 0.0, 1.0);
                Serial.println(buf);
                digitalWrite(MD1_OUT1, HIGH);
                digitalWrite(MD1_OUT2, LOW);
                digitalWrite(MD2_OUT1, HIGH);
                digitalWrite(MD2_OUT2, LOW);
            }
            else if (value == "State: 2")
            {
                sprintf(buf, "GPIO[%d,%d] out1 = 0, out2 = 1;", MD1_OUT1, MD1_OUT2);
                Serial.println(buf);
                sprintf(buf, "P %3.2f: G %3.2f", 1.0, 0.0);
                Serial.println(buf);
                digitalWrite(MD1_OUT1, LOW);
                digitalWrite(MD1_OUT2, HIGH);
                digitalWrite(MD2_OUT1, LOW);
                digitalWrite(MD2_OUT2, HIGH);
            }
            else if (value == "State: 3")
            {
                sprintf(buf, "GPIO[%d,%d] out1 = 1, out2 = 1;", MD1_OUT1, MD1_OUT2);
                Serial.println(buf);
                sprintf(buf, "P %3.2f: G %3.2f", 1.0, 1.0);
                Serial.println(buf);
                digitalWrite(MD1_OUT1, HIGH);
                digitalWrite(MD1_OUT2, HIGH);
                digitalWrite(MD2_OUT1, HIGH);
                digitalWrite(MD2_OUT2, HIGH);
            }
            else
            {
                for (int i = 0; i < value.length(); i++)
                {
                    Serial.print(value[i]);
                }
            }
        }
    }
};

void SetUpBLE()
{
    // Setup Pins
    pinMode(MD1_OUT1, OUTPUT);
    pinMode(MD1_OUT2, OUTPUT);
    pinMode(MD2_OUT1, OUTPUT);
    pinMode(MD2_OUT2, OUTPUT);

    // Create the BLE Device
    BLEDevice::init("ESP32 GET NOTI FROM DEVICE");

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create a BLE Characteristic
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_INDICATE);

    pCharacteristic->setCallbacks(new MyCallbacks());

    // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
    // Create a BLE Descriptor
    pCharacteristic->addDescriptor(new BLE2902());

    // Start the service
    pService->start();

    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0); // set value to 0x00 to not advertise this parameter
    BLEDevice::startAdvertising();
    Serial.println("Waiting a client connection to notify...");
}

void UpdateBLEConnection()
{
    // disconnecting
    if (!deviceConnected && oldDeviceConnected)
    {
        delay(500);                  // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected)
    {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}