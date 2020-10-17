#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#define MD1_IN1 32 //GPIO #32
#define MD1_IN2 33 //GPIO #33
#define MD2_IN1 25 //GPIO #25
#define MD2_IN2 26 //GPIO #26

char buf[100];

class MyServerCallbacks : public BLEServerCallbacks
{
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
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string value = pCharacteristic->getValue();

    if (value.length() > 0)
    {

      Serial.println("**********");

      if (value == "buttonIndex1: 0")
      {
        sprintf(buf, "GPIO[%d,%d] out1 = 0, out2 = 0;", MD1_IN1, MD1_IN2);
        Serial.println(buf);
        digitalWrite(MD1_IN1, LOW);
        digitalWrite(MD1_IN2, LOW);
      }
      else if (value == "buttonIndex1: 1")
      {
        sprintf(buf, "GPIO[%d,%d] out1 = 1, out2 = 0;", MD1_IN1, MD1_IN2);
        Serial.println(buf);
        digitalWrite(MD1_IN1, HIGH);
        digitalWrite(MD1_IN2, LOW);
      }
      else if (value == "buttonIndex1: 2")
      {
        sprintf(buf, "GPIO[%d,%d] out1 = 0, out2 = 1;", MD1_IN1, MD1_IN2);
        Serial.println(buf);
        digitalWrite(MD1_IN1, LOW);
        digitalWrite(MD1_IN2, HIGH);
      }
      else if (value == "buttonIndex1: 3")
      {
        sprintf(buf, "GPIO[%d,%d] out1 = 1, out2 = 1;", MD1_IN1, MD1_IN2);
        Serial.println(buf);
        digitalWrite(MD1_IN1, HIGH);
        digitalWrite(MD1_IN2, HIGH);
      }
      else if (value == "buttonIndex2: 0")
      {
        sprintf(buf, "GPIO[%d,%d] out1 = 0, out2 = 0;", MD2_IN1, MD2_IN2);
        Serial.println(buf);
        digitalWrite(MD2_IN1, LOW);
        digitalWrite(MD2_IN2, LOW);
      }
      else if (value == "buttonIndex2: 1")
      {
        sprintf(buf, "GPIO[%d,%d] out1 = 1, out2 = 0;", MD2_IN1, MD2_IN2);
        Serial.println(buf);
        digitalWrite(MD2_IN1, HIGH);
        digitalWrite(MD2_IN2, LOW);
      }
      else if (value == "buttonIndex2: 2")
      {
        sprintf(buf, "GPIO[%d,%d] out1 = 0, out2 = 1;", MD2_IN1, MD2_IN2);
        Serial.println(buf);
        digitalWrite(MD2_IN1, LOW);
        digitalWrite(MD2_IN2, HIGH);
      }
      else if (value == "buttonIndex2: 3")
      {
        sprintf(buf, "GPIO[%d,%d] out1 = 1, out2 = 1;", MD2_IN1, MD2_IN2);
        Serial.println(buf);
        digitalWrite(MD2_IN1, HIGH);
        digitalWrite(MD2_IN2, HIGH);
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

void setup()
{
  delay(3000);
  Serial.begin(115200);

  // Setup Pins
  pinMode(MD1_IN1, OUTPUT);
  pinMode(MD1_IN2, OUTPUT);
  pinMode(MD2_IN1, OUTPUT);
  pinMode(MD2_IN2, OUTPUT);

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

void loop()
{
  // notify changed value
  //    if (deviceConnected) {
  //        pCharacteristic->setValue((uint8_t*)&value, 4);
  //        pCharacteristic->notify();
  //        value++;
  //        delay(10); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
  //    }
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
  // delay(10000);
}