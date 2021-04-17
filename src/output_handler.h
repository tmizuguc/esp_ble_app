#ifndef OUTPUT_HANDLER_H_
#define OUTPUT_HANDLER_H_

extern void OutputSetup();
extern void HandleOutput(
    float paper_score,
    float rock_score,
    float paper_threshold,
    float rock_threshold);
void OutputType00();
void OutputType10();
void OutputType01();
void OutputType11();

extern float Categorize(
    float value);

#endif // OUTPUT_HANDLER_H_