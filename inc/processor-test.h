#ifndef PROCESSOR_TEST_H
#define PROCESSOR_TEST_H

#include <math.h>
#include "processor.h"
#include <stdlib.h>

Processor* testproc_filter (float start, float stop, int tapbits, int sample_rate);

void testproc_generic_sine_set( Processor* sine, float freq, float amp);
void testproc_generic_sine_get( Processor* sine, float* freq, float* amp);

int testproc_sine440 (float *out, int sample_rate, int length, void *data);
int testproc_sine440_low (float *out, int sample_rate, int length, void *data);
Processor* testproc_generic_sine (float freq, float amp);


#endif