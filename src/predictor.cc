#include <Arduino.h>

#include "predictor.h"

motion Predict(
    float extensor_score,
    float flexor_score,
    float extensor_threshold,
    float flexor_threshold)
{
    if ((extensor_score > extensor_threshold) & (flexor_score > flexor_threshold))
    {
        return NONE;
    }
    else if (extensor_score > extensor_threshold)
    {
        return PAPER;
    }
    else if (flexor_score > flexor_threshold)
    {
        return ROCK;
    }
    return NONE;
}