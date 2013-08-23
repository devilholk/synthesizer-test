#include "processor-test.h"
#include "debug.h"
#include "filter.h"
#include <string.h>

void testproc_generic_sine_set( Processor* sine, float freq, float amp) {
	float *data = (float*) sine->data;
	data[0] = freq;
	data[1] = amp;
}

void testproc_generic_sine_get( Processor* sine, float* freq, float* amp) {
	float *data = (float*) sine->data;
	*freq = data[0];
	*amp = data[1];
	
}

#include <sys/time.h>








typedef struct {
	float start;
	float stop;
	int tapbits;
	int length;
	float *filter;
	float *samples;
	int sample_offset;
} test_filter;


Processor* testproc_filter (float start, float stop, int tapbits, int sample_rate) {

	DEBUG_PRINT( "i början av filter start=%f, tapbits=%i", start, tapbits);
	
	int processor_func(float *out, int sample_rate, int length, void *data) {
	
		test_filter* tf = (test_filter*) data;

		for (int i=0; i!=length; i++) {

			tf->samples[tf->sample_offset] = *out;
			if ((++tf->sample_offset) >= tf->length) tf->sample_offset=0;

			*(out++) = filter_run_fir_iteration( tf->samples, tf->filter, tf->tapbits, tf->sample_offset);

		}
	
				
			
		return 1;
		
	}


	test_filter* tf = (test_filter*) malloc(sizeof(test_filter));

	*tf = (test_filter) {
		.start = start, 
		.stop = stop, 
		.tapbits = tapbits,
		.length = 1<<tapbits,
		.sample_offset = 0
	};
	
	DEBUG_PRINT( "Created filter start=%f, tapbits=%i, length=%i", tf->start, tf->tapbits, tf->length);
	
	tf->filter = malloc(sizeof(float)*tf->length);
	tf->samples = malloc(sizeof(float)*tf->length);
	memset(tf->samples, 0, sizeof(float)*tf->length);

	filter_create_fir(tf->filter, start/(float)sample_rate, tf->tapbits, 1);
	
	Processor* result = processor_create( processor_func, (void*)tf );

	return result;
	
}















Processor* testproc_generic_sine (float freq, float amp) {

	
	int processor_func(float *out, int sample_rate, int length, void *data) {

		float *ptr = (float*)data;
		
		float freq = ptr[0];
		float amp = ptr[1];
		float *t = &ptr[2];
		float *t2 = &ptr[3];
		float *t3 = &ptr[4];
	
		
//		DEBUG_PRINT( "Frequency: %f, sample rate: %i", freq, sample_rate);		

						
		for (int i=0; i!=length; i++) {
			*out++ = sin(*t2) * sin(*t) * amp;  // sin(*t) * amp;
			*t=fmod(*t+((freq * (1.0+sin(*t3)*.13)) * 2.0 * M_PI/sample_rate), 2.0 * M_PI);
			*t2=fmod(*t2+(4.2 * (1.0+sin(*t3)*.27) * 2.0 * M_PI/sample_rate), 2.0 * M_PI);
			*t3=fmod(*t3+(1.23 * 2.0 * M_PI/sample_rate), 2.0 * M_PI);
		}
		return 1;
		
	}

	float* data = malloc(sizeof(float)*5);
	data[0] = freq;
	data[1] = amp;
	data[2] = 0.0;
	data[3] = 0.0;
	data[4] = 0.0;
	Processor* result = processor_create( processor_func, data );

	DEBUG_PRINT ( "Created testproc_generic_sine at 0x%lx with data 0x%lx", (long) result, (long) data);

	return result;
	
}


Processor* testproc_reverb (float freq, float amp) {

	
	int processor_func(float *out, int sample_rate, int length, void *data) {

		float *ptr = (float*)data;
		
		float freq = ptr[0];
		float amp = ptr[1];
		float *t = &ptr[2];
		float *t2 = &ptr[3];
		float *t3 = &ptr[4];
	
		
//		DEBUG_PRINT( "Frequency: %f, sample rate: %i", freq, sample_rate);		

						
		for (int i=0; i!=length; i++) {
			*out++ = sin(*t2) * sin(*t) * amp;  // sin(*t) * amp;
			*t=fmod(*t+((freq * (1.0+sin(*t3)*.13)) * 2.0 * M_PI/sample_rate), 2.0 * M_PI);
			*t2=fmod(*t2+(4.2 * (1.0+sin(*t3)*.27) * 2.0 * M_PI/sample_rate), 2.0 * M_PI);
			*t3=fmod(*t3+(1.23 * 2.0 * M_PI/sample_rate), 2.0 * M_PI);
		}
		return 1;
		
	}

	float* data = malloc(sizeof(float)*5);
	data[0] = freq;
	data[1] = amp;
	data[2] = 0.0;
	data[3] = 0.0;
	data[4] = 0.0;
	Processor* result = processor_create( processor_func, data );

	DEBUG_PRINT ( "Created testproc_generic_sine at 0x%lx with data 0x%lx", (long) result, (long) data);

	return result;
	
}



int testproc_sine440 (float *out, int sample_rate, int length, void *data) {
	static float t = 0.0; 	//Note, if you use two testproc_sine440 you will get 880 Hz instead. This is just for testing
	
	for (int i=0; i!=length; i++) {
		*out++ = sin(t);
		t+=440.0*2.0*M_PI/sample_rate;
	}
	return 1;
}

int testproc_sine440_low (float *out, int sample_rate, int length, void *data) {
	static float t = 0.0; 	//Note, if you use two testproc_sine440_low you will get 880 Hz instead. This is just for testing
	
	for (int i=0; i!=length; i++) {
		*out++ = sin(t) * 0.2;
		t+=440.0*2.0*M_PI/sample_rate;
	}
	return 1;
}