#include <Arduino.h>

#include "input_handler.h"
#include "constants.h"

#define MD_IN1 34 //GPIO #34
#define MD_IN2 35 //GPIO #35

char handle_input_s[64];

bool HandleInput(volatile int r_extensor_data[],
                 volatile int r_flexor_data[],
                 volatile int begin_index,
                 const int r_length)
{

    int a, b;
    b = analogRead(MD_IN1);
    a = analogRead(MD_IN2);
    r_extensor_data[begin_index] = a;
    r_flexor_data[begin_index] = b;

    //TODO: UseML=TrueでONにする
    if (begin_index % 10 == 0)
    {
        unsigned long currentMillis = xTaskGetTickCount();
        sprintf(handle_input_s, "time: %lu\n", currentMillis);
        Serial.println(handle_input_s);
        sprintf(handle_input_s, "index: %d\ne: %d\nf: %d\n", begin_index, r_extensor_data[begin_index], r_flexor_data[begin_index]);
        Serial.println(handle_input_s);
    }

    return true;
}