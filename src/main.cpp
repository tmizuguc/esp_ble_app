#include <Arduino.h>

#include "main.h"
#include "ble.h"
#include "constants.h"
#include "input_handler.h"
#include "signal_processor.h"
#include "output_handler.h"

TaskHandle_t TaskIO;
TaskHandle_t TaskMain;

// セマフォ
SemaphoreHandle_t xMutex = NULL;

char main_s[64];
long last_sample_micros = 0;
long last_process_micros = 0;

// 判定に使用する閾値
int e_threshold = 99999;
int f_threshold = 99999;

int begin_index = 0;
// 筋電センサーからの入力（1000Hz）
int r_extensor_data[r_length] = {0};
int r_flexor_data[r_length] = {0};
// 整列後の筋電センサーからの入力（1000Hz）
int ar_extensor_data[r_length] = {0};
int ar_flexor_data[r_length] = {0};
// 信号処理後の値
float e_score = 0;
float f_score = 0;

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

    if (deviceConnected)
    {
      continue;
    }

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

    if (deviceConnected)
    {
      continue;
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
                  e_score,
                  f_score,
                  r_length);

    // モニター出力
    sprintf(main_s, "idx=[%d]", begin_index);
    Serial.println(main_s);
    sprintf(main_s, "e_sp: %3.2f\n", e_score);
    Serial.println(main_s);
    sprintf(main_s, "f_sp: %3.2f\n", f_score);
    Serial.println(main_s);

    HandleOutput(e_score, f_score, e_threshold, f_threshold);
  }
};

void setup()
{
  delay(3000);
  Serial.begin(115200);

  SetUpBLE();

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