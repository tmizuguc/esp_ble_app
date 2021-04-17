#ifndef OUTPUT_HANDLER_H_
#define OUTPUT_HANDLER_H_

#include "motion.h"

extern void OutputSetup();
extern void HandleOutput(motion motion);
void OutputType00();
void OutputType10();
void OutputType01();
void OutputType11();

extern float Categorize(
    float value);

#endif // OUTPUT_HANDLER_H_