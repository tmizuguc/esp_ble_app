#ifndef PREDICTOR_H_
#define PREDICTOR_H_

#include "motion.h"

extern motion Predict(
    float extensor_score,
    float flexor_score,
    float extensor_threshold,
    float flexor_threshold);

#endif