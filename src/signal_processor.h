#ifndef SIGNAL_PROCESSOR_H_
#define SIGNAL_PROCESSOR_H_

extern long last_process_millis;
extern int process_count;

extern bool SignalProcess(
    int r_extensor_data[],
    int r_flexor_data[],
    volatile float &e_score,
    volatile float &f_score,
    const int r_length);

extern void ArrangeArray(
    volatile int r_extensor_data[],
    volatile int r_flexor_data[],
    int ar_extensor_data[],
    int ar_flexor_data[],
    int begin_index,
    const int r_length);

extern void RollingAverage(
    float a_extensor_data[],
    float a_flexor_data[],
    float m_extensor_data[],
    float m_flexor_data[],
    const int r_length);

extern void Normalization(
    int ar_extensor_data[],
    int ar_flexor_data[],
    float b_extensor_data[],
    float b_flexor_data[],
    const int r_length);

extern float Max(
    float data[],
    int length);

extern float Min(
    float data[],
    int length);

extern int Max(
    int data[],
    int length);

extern int Min(
    int data[],
    int length);

extern String getValue(
    String data,
    char separator,
    int index);

#endif // SIGNAL_PROCESSOR_H_