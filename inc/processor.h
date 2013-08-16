#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "list.h"
typedef int (*ProcessorFunction)(float *out, int sample_rate, int length, void *data);


typedef struct {
	ProcessorFunction function;
	void *data;
} Processor;

Processor* processor_create();
void processor_destroy(Processor* processor);


#endif

