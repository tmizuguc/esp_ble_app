#include <Arduino.h>

#include "main.h"
#include "ble.h"
#include "constants.h"
#include "input_handler.h"
#include "signal_processor.h"
#include "output_handler.h"
#include "motion.h"
#include "predictor.h"
#include "constants_param.h"
#include "constants_param_ml.h"

#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

TaskHandle_t TaskIO;
TaskHandle_t TaskMain;

// セマフォ
SemaphoreHandle_t xMutex = NULL;

char main_s[64];
long last_sample_micros = 0;
long last_process_micros = 0;

// 機械学習モデルを使用するかどうか
// True: 機械学習モデルを使用した判定を行う
// False: 閾値を使用した判定を行う
boolean UseML = false;

// ML: 判定に使用する閾値
float extensor_threshold = 0.7;
float flexor_threshold = 0.7;
// Manual: 判定に使用する閾値
float rock_flexor_lower_limit = 0.0;
float rock_extensor_upper_limit = 0.0;
float paper_extensor_lower_limit = 0.0;
float paper_flexor_upper_limit = 0.0;

int begin_index = 0;
// 筋電センサーからの入力（1000Hz）
int r_extensor_data[r_length] = {0};
int r_flexor_data[r_length] = {0};
// 整列後の筋電センサーからの入力（1000Hz）
int ar_extensor_data[r_length] = {0};
int ar_flexor_data[r_length] = {0};
// 信号処理後の値
volatile float extensor_score = 0;
volatile float flexor_score = 0;

// 初期化関数
void InitData()
{
  std::fill(std::begin(r_extensor_data), std::end(r_extensor_data), 0); // volatileはmemsetできない
  std::fill(std::begin(r_flexor_data), std::end(r_flexor_data), 0);     // volatileはmemsetできない
  memset(ar_extensor_data, 0, r_length * sizeof(int));
  memset(ar_flexor_data, 0, r_length * sizeof(int));
}

// IOスレッド
void TaskIOcode(void *pvParameters)
{
  for (;;)
  {
    // 1000Hz
    if ((micros() - last_sample_micros) < 1 * 1000)
    {
      continue;
    }
    last_sample_micros = micros();

    // ウォッチドッグのために必要
    // https://github.com/espressif/arduino-esp32/issues/3001
    // https://lang-ship.com/blog/work/esp32-freertos-l03-multitask/#toc12
    vTaskDelay(1);

    // 整列処理
    if (xSemaphoreTake(xMutex, (portTickType)100) == pdTRUE)
    {
      begin_index += 1;
      if (begin_index >= r_length - 1)
      {
        begin_index = 0;
      }

      HandleInput(r_extensor_data,
                  r_flexor_data,
                  begin_index,
                  r_length);

      xSemaphoreGive(xMutex);
    }
  }
};

// Mainスレッド
void TaskMaincode(void *pvParameters)
{
  for (;;)
  {
    UpdateBLEConnection();

    // 4Hz
    if ((micros() - last_process_micros) < 250 * 1000)
    {
      continue;
    }
    last_process_micros = micros();

    // ウォッチドッグのために必要
    // https://github.com/espressif/arduino-esp32/issues/3001
    // https://lang-ship.com/blog/work/esp32-freertos-l03-multitask/#toc12
    vTaskDelay(1);

    // Bluetooth
    // if (Serial.available())
    // {
    //   SerialBT.write(Serial.read());
    // }
    if (SerialBT.available())
    {
      // Bluetoothから受け取ったメッセージの処理
      bleCallback();
    }

    if (xSemaphoreTake(xMutex, (portTickType)100) == pdTRUE)
    {
      // Serial.println("taskMain start.");
      // 整列処理
      ArrangeArray(
          r_extensor_data,
          r_flexor_data,
          ar_extensor_data,
          ar_flexor_data,
          begin_index,
          r_length);
      xSemaphoreGive(xMutex);
    }

    // 信号処理
    SignalProcess(ar_extensor_data,
                  ar_flexor_data,
                  extensor_score,
                  flexor_score,
                  r_length);

    // モニター出力
    sprintf(main_s, "idx=[%d]", begin_index);
    Serial.println(main_s);
    sprintf(main_s, "e_sp: %3.2f\n", extensor_score);
    Serial.println(main_s);
    sprintf(main_s, "f_sp: %3.2f\n", flexor_score);
    Serial.println(main_s);

    motion motion = NONE;
    motion = PredictManual(
        extensor_score,
        flexor_score,
        ml_rock_flexor_lower_limit,
        ml_rock_extensor_upper_limit,
        ml_paper_extensor_lower_limit,
        ml_paper_flexor_upper_limit);
    // if (UseML)
    // {
    //   motion = Predict(
    //       extensor_score,
    //       flexor_score,
    //       extensor_threshold,
    //       flexor_threshold);
    // }
    // else
    // {
    //   motion = PredictManual(
    //       extensor_score,
    //       flexor_score,
    //       rock_flexor_lower_limit,
    //       rock_extensor_upper_limit,
    //       paper_extensor_lower_limit,
    //       paper_flexor_upper_limit);
    // }

    HandleOutput(motion);
  }
};

void setup()
{
  delay(3000);
  Serial.begin(115200);

  SerialBT.begin("ESP32_CLASSIC_BT");
  Serial.println("The device started, now you can pair it with bluetooth!");

  xMutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(TaskIOcode, "TaskIO", 4096, NULL, 2, &TaskIO, 0); //Task1実行
  delay(500);
  xTaskCreatePinnedToCore(TaskMaincode, "TaskMain", 4096, NULL, 2, &TaskMain, 1); //Task2実行
  delay(500);

  Serial.println("[setup] finished.");
}

void loop()
{
  // loop(デフォルトでCPU1)は使用しないので、削除する
  vTaskDelete(NULL);
}

// Bluetoothからデータ送信された際に呼び出される関数
void bleCallback()
{
  String receiveData = SerialBT.readStringUntil(';');
  String type = getValue(receiveData, ':', 0);
  if (type == "useML")
  {
    Serial.println("UseML is True");
    UseML = true;
  }
  if (type == "useManual")
  {
    Serial.println("UseML is False");
    UseML = false;
  }
  if (type == "Rock")
  {
    Serial.println("ExtensorThreshold is Changed");
    if (UseML)
    {
      String value = getValue(receiveData, ':', 1);
      flexor_threshold = flexor_max * value.toFloat();

      sprintf(main_s, "FlexorThreshold: %3.2f\n", value.toFloat());
      Serial.println(main_s);
    }
    else
    {
      String ll_value = getValue(receiveData, ':', 1);
      String ul_value = getValue(receiveData, ':', 2);
      rock_flexor_lower_limit = ll_value.toFloat();
      rock_extensor_upper_limit = ul_value.toFloat();

      sprintf(main_s, "RockFlexorLowerLimit: %3.2f\n", rock_flexor_lower_limit);
      Serial.println(main_s);
      sprintf(main_s, "RockExtensorUpperLimit: %3.2f\n", rock_extensor_upper_limit);
      Serial.println(main_s);
    }
  }
  if (type == "Paper")
  {
    Serial.println("FlexorThreshold is Changed");
    if (UseML)
    {
      String value = getValue(receiveData, ':', 1);
      extensor_threshold = extensor_max * value.toFloat();

      sprintf(main_s, "ExtensorThreshold: %3.2f\n", value.toFloat());
      Serial.println(main_s);
    }
    else
    {
      String ll_value = getValue(receiveData, ':', 1);
      String ul_value = getValue(receiveData, ':', 2);
      paper_extensor_lower_limit = ll_value.toFloat();
      paper_flexor_upper_limit = ul_value.toFloat();

      sprintf(main_s, "PaperExtensorLowerLimit: %3.2f\n", paper_extensor_lower_limit);
      Serial.println(main_s);
      sprintf(main_s, "PaperFlexorUpperLimit: %3.2f\n", paper_flexor_upper_limit);
      Serial.println(main_s);
    }
  }
}