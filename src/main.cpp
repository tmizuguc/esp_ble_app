#include <Arduino.h>

#include "main.h"
#include "ble.h"

void setup()
{
  delay(3000);
  Serial.begin(115200);

  SetUpBLE();
}

void loop()
{
  UpdateBLEConnection();

  // if (deviceConnected)
  // {
  //   Serial.println("connect");
  //   delay(1000);
  // }
  // else
  // {
  //   Serial.println("unconnect");
  //   delay(1000);
  // }
}