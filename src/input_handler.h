#ifndef INPUT_HANDLER_H_
#define INPUT_HANDLER_H_

extern bool HandleInput(volatile int r_extensor_data[],
                        volatile int r_flexor_data[],
                        volatile int begin_index,
                        const int r_length);

#endif // INPUT_HANDLER_H_