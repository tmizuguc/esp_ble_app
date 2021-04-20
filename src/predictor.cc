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

motion PredictManual(
    float extensor_score,
    float flexor_score,
    float rock_flexor_lower_limit,
    float rock_extensor_upper_limit,
    float paper_extensor_lower_limit,
    float paper_flexor_upper_limit)
{
    if ((extensor_score > paper_extensor_lower_limit) & (flexor_score < paper_flexor_upper_limit))
    {
        return PAPER;
    }
    else if ((flexor_score > rock_flexor_lower_limit) & (extensor_score < rock_extensor_upper_limit))
    {
        return ROCK;
    }
    return NONE;
}