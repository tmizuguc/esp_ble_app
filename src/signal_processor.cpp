#include <Arduino.h>
#include <stdlib.h>

#include "signal_processor.h"
#include "constants.h"
#include "filter.cpp"

char sp_s[64];

float *b_extensor_data = NULL;
float *b_flexor_data = NULL;

float *m_extensor_data = NULL;
float *m_flexor_data = NULL;

const float extensor_ave = 1437;
const float flexor_ave = 1293;
const float extensor_max = 1;
const float flexor_max = 49;
const float extensor_min = 0;
const float flexor_min = 17;

CMyFilter filter;

// ノッチフィルタのパラメータ
float notch_freq = 50.0f;
float notch_bw = 1.0f;

// バンドパスフィルタのパラメータ
// float bandpass_freq = 100.0f;
// float bandpass_bw = 2.0f;

bool SignalProcess(int ar_extensor_data[],
                   int ar_flexor_data[],
                   float e_score,
                   float f_score,
                   const int r_length)
{
    // 1.正規化+ABS
    b_extensor_data = (float *)pvPortMalloc(sizeof(float) * r_length);
    b_flexor_data = (float *)pvPortMalloc(sizeof(float) * r_length);
    Normalization(
        ar_extensor_data,
        ar_flexor_data,
        b_extensor_data,
        b_flexor_data,
        r_length);

    // 2.移動平均
    m_extensor_data = (float *)pvPortMalloc(sizeof(float) * r_length);
    m_flexor_data = (float *)pvPortMalloc(sizeof(float) * r_length);
    RollingAverage(
        b_extensor_data,
        b_flexor_data,
        m_extensor_data,
        m_flexor_data,
        r_length);
    vPortFree(b_extensor_data);
    vPortFree(b_flexor_data);

    // モニター出力
    unsigned long currentMillis = xTaskGetTickCount();
    sprintf(sp_s, "time: %lu\n", currentMillis);
    sprintf(sp_s, "e_sp: %3.2f\n", b_extensor_data[-1]);
    sprintf(sp_s, "f_sp: %3.2f\n", b_flexor_data[-1]);

    return true;
}

/**
 * 整列
 */
void ArrangeArray(
    volatile int r_extensor_data[],
    volatile int r_flexor_data[],
    int ar_extensor_data[],
    int ar_flexor_data[],
    int begin_index,
    const int r_length)
{
    for (int i = 0; i < r_length; ++i)
    {
        int ring_array_index = begin_index + i - r_length;
        if (ring_array_index < 0)
        {
            ring_array_index += r_length;
        }
        ar_extensor_data[i] = r_extensor_data[ring_array_index];
        ar_flexor_data[i] = r_flexor_data[ring_array_index];
    }
}

/**
 * フィルタ＋正規化
 */
void Normalization(
    int ar_extensor_data[],
    int ar_flexor_data[],
    float b_extensor_data[],
    float b_flexor_data[],
    const int r_length)
{
    for (int i = 0; i < r_length; ++i)
    {

        float f_extensor = (float)ar_extensor_data[i];
        float f_flexor = (float)ar_flexor_data[i];

        // ノッチフィルタ
        filter.Notch(notch_freq, notch_bw, kTargetHz);
        f_extensor = (short)filter.Process(f_extensor);
        f_flexor = (short)filter.Process(f_flexor);

        // バンドパスフィルタ
        // filter.BandPass(bandpass_freq, bandpass_bw, kTargetHz);
        // f_extensor = (short)filter.Process(f_extensor);
        // f_flexor = (short)filter.Process(f_flexor);

        // 正規化（aveは学習によって決定）
        b_extensor_data[i] = abs(f_extensor - extensor_ave);
        b_flexor_data[i] = abs(f_flexor - flexor_ave);
    }
}

/**
 * 移動平均
 */
void RollingAverage(
    float b_extensor_data[],
    float b_flexor_data[],
    float m_extensor_data[],
    float m_flexor_data[],
    const int r_length)
{
    float extensor_sum = 0;
    float flexor_sum = 0;
    int m_i = 0;

    // 100msのフィルタ
    int filterWidth = kWindowWidth / 10;

    for (int i = 0; i < filterWidth; i++)
    {
        extensor_sum += b_extensor_data[i];
        flexor_sum += b_flexor_data[i];
        if (i >= (filterWidth / 2) - 1)
        {
            m_extensor_data[m_i] = extensor_sum / (i + 1);
            m_flexor_data[m_i] = flexor_sum / (i + 1);
            m_i += 1;
        }
    }

    for (int i = filterWidth; i < r_length; i++)
    {
        extensor_sum -= b_extensor_data[i - filterWidth];
        extensor_sum += b_extensor_data[i];
        m_extensor_data[m_i] = extensor_sum / filterWidth;
        flexor_sum -= b_flexor_data[i - filterWidth];
        flexor_sum += b_flexor_data[i];
        m_flexor_data[m_i] = flexor_sum / filterWidth;
        m_i += 1;
    }

    for (int i = r_length; i < r_length + (filterWidth / 2); i++)
    {
        extensor_sum -= b_extensor_data[i - filterWidth];
        m_extensor_data[m_i] = extensor_sum / (filterWidth - (i - r_length) + 1);
        flexor_sum -= b_flexor_data[i - filterWidth];
        m_flexor_data[m_i] = flexor_sum / (filterWidth - (i - r_length) + 1);
        m_i += 1;
        if (m_i >= r_length)
        {
            break;
        }
    }

    // フィルタ後の正規化（max,minは学習によって決定）
    for (int i = 0; i < r_length; i++)
    {
        m_extensor_data[i] = (m_extensor_data[i] - extensor_min) / (extensor_max - extensor_min);
        m_flexor_data[i] = (m_flexor_data[i] - flexor_min) / (flexor_max - flexor_min);
    }
}
