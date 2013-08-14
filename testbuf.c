#include <stdio.h>

typedef struct {
	float *ptr;
	int head;
	int tail;
	int length;
} circ_buf_ptr;

float test_reverb_l[5] = {[0 ... 4] = 0.0};
circ_buf_ptr test_reverb_l_ptr = {.ptr=test_reverb_l, .length=5, .head=0, .tail=4};

int main(void) {

		for (int i=0;i!=10;i++) {
			
			printf("Buffer: ");			
			for (int b=0;b!=5;b++) {
				printf("[%f] ", test_reverb_l[b]);
			}			
			
			float test=(float)i;
			float *sample_L = &test;

			test_reverb_l_ptr.ptr[test_reverb_l_ptr.head] = *sample_L;
			*sample_L = test_reverb_l_ptr.ptr[test_reverb_l_ptr.tail]; 

			printf("head: %i tail: %i  sample: %f\n", test_reverb_l_ptr.head, test_reverb_l_ptr.tail, *sample_L);

			
			if (++test_reverb_l_ptr.head >= test_reverb_l_ptr.length) test_reverb_l_ptr.head = 0;  
			if (++test_reverb_l_ptr.tail >= test_reverb_l_ptr.length) test_reverb_l_ptr.tail = 0;  
		}

	return(0);
}