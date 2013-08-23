#include "processor.h"
#include "debug.h"
#include <stdlib.h>

Processor* processor_create(ProcessorFunction function, void *data) {
	Processor* result = malloc(sizeof(Processor));
	DEBUG_PRINT ( "Created Processor at 0x%lx with data 0x%lx", (long) result, (long) data);

	*result = (Processor) {.function = function, .data = data };
	return result;
}

void processor_destroy(Processor* processor) {
	DEBUG_PRINT ( "Destroyed Processor at 0x%lx", (long) processor );
	free(processor->data);
	free(processor);
}

