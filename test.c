#include <stdlib.h>
typedef struct {
        int buffersize;
        int length;
        float feedback;
        float* buffer;
} reverb_data;

int main(void) {


reverb_data* data;

*data = (reverb_data) {.buffersize = (int) 10};
}
