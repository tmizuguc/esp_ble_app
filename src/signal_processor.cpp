#include <Arduino.h>
#include <stdlib.h>

#include "signal_processor.h"
#include "constants_param.h"
#include "constants.h"
#include "filter.cpp"

char sp_s[64];

float *b_extensor_data = NULL;
float *b_flexor_data = NULL;

float *m_extensor_data = NULL;
float *m_flexor_data = NULL;

CMyFilter filter;

// ノッチフィルタのパラメータ
// float notch_freq = 50.0f;
// float notch_bw = 1.0f;

// バンドパスフィルタのパラメータ
// float bandpass_freq = 100.0f;
// float bandpass_bw = 2.0f;

bool SignalProcess(int ar_extensor_data[],
                   int ar_flexor_data[],
                   volatile float &extensor_score,
                   volatile float &flexor_score,
                   const int r_length)
{
    // 1.正規化+ABS
    b_extensor_data = (float *)pvPortMalloc(sizeof(float) * r_length);
    b_flexor_data = (float *)pvPortMalloc(sizeof(float) * r_length);

    float extensor_mean = Mean(ar_extensor_data, r_length);
    float flexor_mean = Mean(ar_flexor_data, r_length);
    Normalization(
        ar_extensor_data,
        ar_flexor_data,
        b_extensor_data,
        b_flexor_data,
        extensor_mean,
        flexor_mean,
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

    // 結果
    extensor_score = Max(m_extensor_data, r_length);
    flexor_score = Max(m_flexor_data, r_length);
    vPortFree(m_extensor_data);
    vPortFree(m_flexor_data);

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
    float extensor_mean,
    float flexor_mean,
    const int r_length)
{
    for (int i = 0; i < r_length; ++i)
    {

        float f_extensor = (float)ar_extensor_data[i];
        float f_flexor = (float)ar_flexor_data[i];

        // ノッチフィルタ
        // filter.Notch(notch_freq, notch_bw, kTargetHz);
        // f_extensor = (short)filter.Process(f_extensor);
        // f_flexor = (short)filter.Process(f_flexor);

        // バンドパスフィルタ
        // filter.BandPass(bandpass_freq, bandpass_bw, kTargetHz);
        // f_extensor = (short)filter.Process(f_extensor);
        // f_flexor = (short)filter.Process(f_flexor);

        // 正規化
        b_extensor_data[i] = f_extensor;
        b_flexor_data[i] = f_flexor;
        b_extensor_data[i] = abs(f_extensor - extensor_mean);
        b_flexor_data[i] = abs(f_flexor - flexor_mean);
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

    // 幅100(=1ms*100=100ms)のフィルタ
    int filterWidth = kWindowWidth / 100;

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
}

// Max, Min取得（別ファイルにしたい）
float Max(
    float data[],
    int length)
{
    float cur_max = -99999;
    for (int i = 0; i < length; i++)
    {
        if (cur_max < data[i])
        {
            cur_max = data[i];
        }
    }
    return cur_max;
}

float Min(
    float data[],
    int length)
{
    float cur_min = 99999;
    for (int i = 0; i < length; i++)
    {
        if (cur_min > data[i])
        {
            cur_min = data[i];
        }
    }
    return cur_min;
}

int Max(
    int data[],
    int length)
{
    int cur_max = -99999;
    for (int i = 0; i < length; i++)
    {
        if (cur_max < data[i])
        {
            cur_max = data[i];
        }
    }
    return cur_max;
}

int Min(
    int data[],
    int length)
{
    int cur_min = 99999;
    for (int i = 0; i < length; i++)
    {
        if (cur_min > data[i])
        {
            cur_min = data[i];
        }
    }
    return cur_min;
}

int Mean(
    int data[],
    int length)
{
    int total = 0;
    for (int i = 0; i < length; i++)
    {
        total += data[i];
    }
    float mean_value = (float)total / (float)length;
    return mean_value;
}

// 文字列分割
String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++)
    {
        if (data.charAt(i) == separator || i == maxIndex)
        {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}