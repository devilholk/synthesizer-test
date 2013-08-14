
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "portaudio.h"
#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 256
#include <unistd.h>
#include <termios.h>

#include <Python.h>

int start_synth(void);
int stop_synth(void);
int destroy_synth_audio(void);
int init_synth_audio(void);

typedef struct {
	int head;
	int tail;
	int length;
} circ_buf_ptr;

//float test_reverb_l[8192];// = {[0 ... 8191] = 0.0};
//circ_buf_ptr test_reverb_l_ptr = {.ptr=test_reverb_l, .length=8192, .head=0, .tail=200};

typedef struct {
	float *buf;
	circ_buf_ptr ptr;	
} reverb;


reverb* create_reverb(int bufsize, int tailpos) {
	reverb* res = malloc(sizeof(reverb));
	res->buf = malloc(sizeof(float)*bufsize);
	res->ptr = (circ_buf_ptr){.head=0, .tail=tailpos, .length=bufsize};
	return(res);
}


reverb* test_reverb = NULL;

typedef struct {
	float start;
	float end;
	float time;
} ramp;

typedef struct {
	ramp* amplitude;
	ramp* frequency;	
	int amplitude_len;
	int frequency_len;
	float amplitude_elapsed;
	float frequency_elapsed;
	int amplitude_index;
	int frequency_index;
	float phase;
	float (*funcgen)(float);
}  boing;


boing* create_note(float note, float amplitude, float(*funcgen)(float) , float attack);
boing* create_short_note(float note, float amplitude, float(*funcgen)(float) );
boing* create_bass_drum(float note, float amplitude, float(*funcgen)(float) );


float Tri(float phase) {
	return (fmod(1.0+phase / M_PI,2.0)-1.0);
}

float Sin(float phase) {
	return (sin(phase));
}

float Noise() {
	return (float)rand()/(float)RAND_MAX;
}

float semi_note(float note) {
	return(27.5*pow(2.0, note / 12.0));
}


struct slist_item {
	void* item;
	struct slist_item* prev;
	struct slist_item* next;
};

typedef struct slist_item list_item;

typedef struct {
	list_item *first;
	list_item *last;
} list;


list livenotes = {.first=NULL, .last=NULL};

void add_item(list *List, void *item);


static PyObject *synth_create_reverb(PyObject *self, PyObject *args) {
	int bufsize=0, tailpos=0;

    if (!PyArg_ParseTuple(args, "ii",&bufsize, &tailpos))
        return NULL;
           
    test_reverb = create_reverb(bufsize, tailpos); 

    return Py_BuildValue("");
}



static PyObject *synth_add_note(PyObject *self, PyObject *args) {
	float note=0.0, vel=0.0;
	int chan=0;
    if (!PyArg_ParseTuple(args, "iff",&chan, &note, &vel))
        return NULL;
             
   switch(chan) {
   	case 0:
		add_item(&livenotes, (void*) create_note(note, vel, Sin, 0.05));
		break;
	case 1:	
		add_item(&livenotes, (void*) create_note(note, vel, Tri, 0.05));
		break;
	case 2:
		add_item(&livenotes, (void*) create_short_note(note, vel, Sin));
		break;
	case 3:
		add_item(&livenotes, (void*) create_short_note(note, vel, Tri));
		break;
	case 4:
		add_item(&livenotes, (void*) create_short_note(note, vel, Noise));
		break;
	case 5:
		add_item(&livenotes, (void*) create_bass_drum(note, vel, Tri));
		break;
	case 6:
		add_item(&livenotes, (void*) create_bass_drum(note, vel, Sin));
		break;
	case 7:	
		add_item(&livenotes, (void*) create_note(note, vel, Sin, 0.4));
		break;
	case 8:	
		add_item(&livenotes, (void*) create_note(note, vel, Tri, 0.4));
		break;
	}

    return Py_BuildValue("");
}

static PyObject *
synth_start(PyObject *self, PyObject *args) {
             
	init_synth_audio();
    return Py_BuildValue("i", start_synth());
}

//int start_synth(void)
//int stop_synth(void) {
//int destroy_synth_audio(void) {



static PyMethodDef SynthMethods[] = {
    {"play_note",  synth_add_note, METH_VARARGS, "Play a note"},
    {"init",  synth_start, METH_VARARGS, "Setup audio and start synthesizer"},
    {"create_reverb",  synth_create_reverb, METH_VARARGS, "Create reverb"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};


PyMODINIT_FUNC initsynth(void) {
    (void) Py_InitModule("synth", SynthMethods);
//	init_synth_audio();
    
}






void print_ramp(ramp *data) {
	printf("ramp %f - %f, time = %f\n", data->start, data->end, data->time);
}

void boing_init(boing* data) {
	data->amplitude_index = 0;
	data->frequency_index = 0;
	data->amplitude_elapsed = 0.0;
	data->frequency_elapsed = 0.0;
	data->phase=0.0;
	//print_ramp(&data->amplitude[0]);
	//print_ramp(&data->frequency[0]);
}

typedef struct {
	int (*handler)(void*, float*, int, int);
	void* data;
} instrument;

float ramp_calc(ramp* data, float elapsed) {
	if (data->time==0.0) return(data->start);
	return ((data->end - data->start) * elapsed / data->time + data->start);
}


int boing_think(void* idata, float* out, int sample_rate, int length) {
	boing *data=(boing*)idata;
//	printf("elapsed: %f sample-rate: %u\n", data->amplitude_elapsed, sample_rate);
//printf("amplitude index: %i, frequency index: %i\n", data->amplitude_index, data->frequency_index);
	float sample_time = 1.0 / (float)sample_rate;
	for (int i=0;i!=length;i++) {
		float amp = ramp_calc(&data->amplitude[data->amplitude_index], data->amplitude_elapsed);
		float freq = ramp_calc(&data->frequency[data->frequency_index], data->frequency_elapsed);
		float sample = data->funcgen(data->phase)*amp;
		(*out++)+= sample;
		(*out++)+= sample;
				
		data->phase+=freq*2.0*M_PI/sample_rate;
				
		data->amplitude_elapsed += sample_time;
		data->frequency_elapsed += sample_time;
		if ((data->amplitude_elapsed >= data->amplitude[data->amplitude_index].time) && (data->amplitude[data->amplitude_index].time!=0.0)) {
			if (++data->amplitude_index >= data->amplitude_len) return(0);
			data->amplitude_elapsed=0;
		}
		if ((data->frequency_elapsed >= data->frequency[data->frequency_index].time) && (data->frequency[data->frequency_index].time!=0.0)) {
			if (++data->frequency_index >= data->frequency_len) return(0);
			data->frequency_elapsed=0;
		}
	}
	return(1);
}


/*void test_note(float note) {
	float f=semi_note(note);
	boing ins = {
		.amplitude = (ramp[]) {
			{.start = 0.0, .end = 0.2, .time = 0.01},
			{.start = 0.2, .end = 0.1, .time = 0.1},
			{.start = 0.1, .end = 0.0, .time = 1.0}
		},
		.amplitude_len = 3,
		.frequency = (ramp[]) {
			{.start = f, .end = *1.05, .time = 0.1},
			{.start = f*1.05, .end = f, .time = 0.1},
			{.start = f, .end = *1.025, .time = 0.1},
			{.start = f*1.025, .end = f, .time = 0.1},
			{.start = f, .end = 0.0, .time = 0.0},
		},
		.frequency_len = 5,
		.funcgen = tri
	};
	

}
*/



void free_boing(boing* data) {
	free(data->amplitude);
	free(data->frequency);
	free(data);
}

boing* create_note(float note, float amplitude, float(*funcgen)(float) , float attack) {

	float f=semi_note(note);

 	ramp *amp = (ramp*)malloc(3 * sizeof(ramp));
 	ramp *freq = (ramp*)malloc(5 * sizeof(ramp));
 	boing *res = (boing*)malloc(sizeof(boing));

	amp[0]=(ramp){.start = 0.0, .end = amplitude, .time = attack};
	amp[1]=(ramp){.start = amplitude, .end = 0.05, .time = 0.5};
	amp[2]=(ramp){.start = 0.05, .end = 0.0, .time = 0.5};

	freq[0]=(ramp){.start = f, .end = f*1.04, .time = 0.1};
	freq[1]=(ramp){.start = f*1.04, .end = f, .time = 0.1};
	freq[2]=(ramp){.start = f, .end = f*1.02, .time = 0.1};
	freq[3]=(ramp){.start = f*1.02, .end = f, .time = 0.1};
	freq[4]=(ramp){.start = f, 0.0, .time = 0.0};

	res->amplitude=amp;
	res->frequency=freq;
	res->amplitude_len=3;
	res->frequency_len=5;
	res->funcgen=funcgen;
	boing_init(res);
	return(res);
	
}



boing* create_short_note(float note, float amplitude, float(*funcgen)(float) ) {

	float f=semi_note(note);

 	ramp *amp = (ramp*)malloc(3 * sizeof(ramp));
 	ramp *freq = (ramp*)malloc(5 * sizeof(ramp));
 	boing *res = (boing*)malloc(sizeof(boing));

	amp[0]=(ramp){.start = 0.0, .end = amplitude, .time = 0.01};
	amp[1]=(ramp){.start = amplitude, .end = 0.0, .time = 0.2};

	freq[0]=(ramp){.start = f, 0.0, .time = 0.0};

	res->amplitude=amp;
	res->frequency=freq;
	res->amplitude_len=2;
	res->frequency_len=1;
	res->funcgen=funcgen;
	boing_init(res);
	return(res);
	
}
boing* create_bass_drum (float note, float amplitude, float(*funcgen)(float) ) {

	float f=semi_note(note);

 	ramp *amp = (ramp*)malloc(2 * sizeof(ramp));
 	ramp *freq = (ramp*)malloc(2 * sizeof(ramp));
 	boing *res = (boing*)malloc(sizeof(boing));

	amp[0]=(ramp){.start = 0.0, .end = amplitude, .time = 0.001};
	amp[1]=(ramp){.start = amplitude, .end = 0.0, .time = 0.2};

	freq[0]=(ramp){.start = f, 0.0, .time = 0.0};
	freq[1]=(ramp){.start = f*0.1, 0.2, .time = 0.0};

	res->amplitude=amp;
	res->frequency=freq;
	res->amplitude_len=2;
	res->frequency_len=2;
	res->funcgen=funcgen;
	boing_init(res);
	return(res);
	
}












void del_item(list* List, list_item *item) {

	if (List->first == item) List->first = item->next; 	
	if (List->last == item) List->last = item->prev; 	
	if (item->next) item->next->prev = item->prev;
	if (item->prev) item->prev->next = item->next;
	free(item->item);
	free(item);

//	printf("Item to remove: %u\n", item);
//	printf("Before delete, first=%u, last=%u\n", (int)List->first, (int)List->last);
/*	if (List->first) printf("  first->next %u, first->prev %u\n", (int)List->first->next, (int)List->first->prev);
	if (List->last) printf("  last->next %u, last->prev %u\n", (int)List->last->next, (int)List->last->prev);
	
	if (List->first==item) List->first=item->next;	//Update first
	if (List->last==item) List->last=item->prev;		//update last
	if (item->prev) item->prev=item->next; //Remove the entry unless it is the first one in which case it got removed already
	if (item->next) item->next=item->prev;
	//Free memory
	free(item->item);
	free(item);
//	printf("After delete, first=%u, last=%u\n", (int)List->first, (int)List->last);
	if (List->first) printf("  first->next %u, first->prev %u\n", (int)List->first->next, (int)List->first->prev);
	if (List->last) printf("  last->next %u, last->prev %u\n", (int)List->last->next, (int)List->last->prev);
*/}

void add_item(list *List, void *item) {
	list_item *list_entry = malloc(sizeof(list_item));
	if (List->last) {	//Linked list is not empty, append to the end
	//printf("to end\n");
		*list_entry=(list_item){.item=item, .prev=List->last, .next=NULL};
		List->last->next = list_entry;
		List->last = list_entry;	//Update last	
	} else {
//	printf("new lsit\n");
		*list_entry=(list_item){.item=item, .prev=NULL, .next=NULL};
		List->first = list_entry; //Point both first and last to this one
		List->last = list_entry;		
	}
}









static int liveplayCallback( const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData ) {

	float *out = (float*)outputBuffer;
	
	(void) timeInfo; /* Prevent unused variable warnings. */
	(void) statusFlags;
	(void) inputBuffer;

	for (int i=0;i!=framesPerBuffer<<1;i++) *out++=0.0; 

	out = (float*)outputBuffer;
	


	if (userData) {
		list* note_list = userData;
		list_item* note=note_list->first;
		while (note) {
			boing* note_data = (boing*)note->item;
			if (!boing_think(note_data, out, SAMPLE_RATE, framesPerBuffer)) {
				//Remove this note
				list_item *next = note->next;
				del_item(note_list, note);
				note=next;

			} else {
				note=note->next;
			}
		}
	}

	//eko här	


//	printf("head: %i tail: %i length: %i\n", test_reverb_l_ptr.head, test_reverb_l_ptr.tail, test_reverb_l_ptr.length );


	if (test_reverb) {
		out = (float*)outputBuffer;
		for (int i=0;i!=(framesPerBuffer);i++) {
			float *sample_L = out++;
			float *sample_R = out++;


			*sample_L = 
test_reverb->buf[test_reverb->ptr.head] =
 *sample_L * 0.9 + test_reverb->buf[test_reverb->ptr.tail] *0.3;

//			 test_reverb->buf[test_reverb->ptr.head]=*sample_L;
			//*sample_L = test_reverb->buf[test_reverb->ptr.tail];


			if (++test_reverb->ptr.head >= test_reverb->ptr.length) test_reverb->ptr.head = 0;  
			if (++test_reverb->ptr.tail >= test_reverb->ptr.length) test_reverb->ptr.tail = 0;  





	
	/*		test_reverb_l_ptr.ptr[test_reverb_l_ptr.head] = *sample_L;
			*sample_L = test_reverb_l_ptr.ptr[test_reverb_l_ptr.tail]; 
	*/		
	
	//		test_reverb_l_ptr.ptr[test_reverb_l_ptr.head] = *sample_L;
		//	*sample_L = test_reverb_l_ptr.ptr[test_reverb_l_ptr.tail]; 
	
	
			*sample_R = *sample_L;
	
		//	if (++test_reverb_l_ptr.head >= test_reverb_l_ptr.length) test_reverb_l_ptr.head = 0;  
	//		if (++test_reverb_l_ptr.tail >= test_reverb_l_ptr.length) test_reverb_l_ptr.tail = 0;  
	
	
	//		*sample_L = test_reverb_l_ptr.ptr[test_reverb_l_ptr.head] = test_reverb_l_ptr.ptr[test_reverb_l_ptr.tail] * 0.76 + *sample_L * 0.9;
			//*sample_R = test_reverb_r_ptr.ptr[test_reverb_r_ptr.head] = test_reverb_r_ptr.ptr[test_reverb_r_ptr.tail] * 0.76 + *sample_R * 0.9;
			
	//		printf( "[LEFT head: %i, tail: %i, len: %i, ptr: 0x%x]   ", test_reverb_l_ptr.head,test_reverb_l_ptr.tail, test_reverb_l_ptr.length,(int) test_reverb_l_ptr.ptr); 		
			//printf( "[RIGHT head: %i, tail: %i, len: %i, ptr: 0x%x]   ", test_reverb_r_ptr.head,test_reverb_r_ptr.tail, test_reverb_r_ptr.length,(int) test_reverb_r_ptr.ptr); 		
			
	/*		if (++test_reverb_l_ptr.head >= test_reverb_l_ptr.length) test_reverb_l_ptr.head = 0;  
			if (++test_reverb_l_ptr.tail >= test_reverb_l_ptr.length) test_reverb_l_ptr.tail = 0;  
			if (++test_reverb_r_ptr.head >= test_reverb_r_ptr.length) test_reverb_r_ptr.head = 0;  
			if (++test_reverb_r_ptr.tail >= test_reverb_r_ptr.length) test_reverb_r_ptr.tail = 0;  
	*/		
	
		}
		  // {.start=&test_reverb_l[0], .end=&test_reverb_l[8191], .head=&test_reverb_l[0], .tail=&test_reverb_l[0]};
	}	
	
	
	return paContinue;
}
	

static int patestCallback( const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData ) {

	float *out = (float*)outputBuffer;
	
	(void) timeInfo; /* Prevent unused variable warnings. */
	(void) statusFlags;
	(void) inputBuffer;

	for (int i=0;i!=framesPerBuffer<<1;i++) *out++=0.0;

	out = (float*)outputBuffer;

	if (userData) {
		instrument* ins = userData;
		if (ins->handler) {
			if (!ins->handler(ins->data, out, SAMPLE_RATE, framesPerBuffer)) {
				ins->handler = NULL;
			}
			return paContinue;
		}
	}
	return paComplete;
}






char getch() {
        char buf = 0;
        struct termios old = {0};
        if (tcgetattr(0, &old) < 0)
                perror("tcsetattr()");
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSANOW, &old) < 0)
                perror("tcsetattr ICANON");
        if (read(0, &buf, 1) < 0)
                perror ("read()");
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        if (tcsetattr(0, TCSADRAIN, &old) < 0)
                perror ("tcsetattr ~ICANON");
        return (buf);
}


/*
 * This routine is called by portaudio when playback is done.
 */
static void StreamFinished( void* userData )
{
 printf( "Stream Completed\n");
}

PaStreamParameters outputParameters;
PaStream *stream;


int init_synth_audio(void) {
	PaError err;
	printf("Initiera pa\n");
	err = Pa_Initialize();
	printf("err = %i", err);
 	if( err != paNoError ) return err;



 	return 0;
}

int destroy_synth_audio(void) {
	Pa_Terminate();
 	return 0;
}	

int stop_synth(void) {
PaError err;
	err = Pa_StopStream( stream );
	if( err != paNoError ) return err;
	
	err = Pa_CloseStream( stream );
	if( err != paNoError ) return err;

//free_boing(note);
 	return 0;

}

int start_synth(void) {

// boing* note =create_note(10, 0.2, Tri);

// instrument ins={.handler = boing_think, .data = (void*)note};


 outputParameters.device = Pa_GetDefaultOutputDevice(); // default output device 
 if (outputParameters.device == paNoDevice) {
 fprintf(stderr,"Error: No default output device.\n");
return -1;
 }
 outputParameters.channelCount = 2; // stereo output 
 outputParameters.sampleFormat = paFloat32; // 32 bit floating point output 
 outputParameters.suggestedLatency = 0.01;//Pa_GetDeviceInfo( outputParameters.device )->defaultHighOutputLatency;
 outputParameters.hostApiSpecificStreamInfo = NULL;
PaError err;


 err = Pa_OpenStream(
	 &stream,
 	NULL, // no input 
 	&outputParameters,
 	SAMPLE_RATE,
 	FRAMES_PER_BUFFER,
 	paNoFlag, // we won't output out of range samples so don't bother clipping them 
 	liveplayCallback,
 	(void*)&livenotes
 );
 if( err != paNoError ) return err;

 err = Pa_SetStreamFinishedCallback( stream, &StreamFinished );
 if( err != paNoError ) return err;

 err = Pa_StartStream( stream );
 if( err != paNoError ) return err;

// printf("Play for %d seconds.\n", NUM_SECONDS );
// Pa_Sleep( NUM_SECONDS * 1000 );


/*while(Pa_IsStreamActive(stream)) {
	switch(getch()) {
		case 'a':
			add_item(&livenotes, (void*) create_note(12+15, 0.2, Tri));
			break;	
		case 's':
			add_item(&livenotes, (void*) create_note(12+17, 0.2, Tri));
			break;	
		case 'd':
			add_item(&livenotes, (void*) create_note(12+19, 0.2, Tri));
			break;	
		case 'f':
			add_item(&livenotes, (void*) create_note(12+20, 0.2, Tri));
			break;	
		case 'g':
			add_item(&livenotes, (void*) create_note(12+22, 0.2, Tri));
			break;	
		case 'h':
			add_item(&livenotes, (void*) create_note(12+24, 0.2, Tri));
			break;	
		case 'j':
			add_item(&livenotes, (void*) create_note(12+26, 0.2, Tri));
			break;	
		case 'k':
			add_item(&livenotes, (void*) create_note(12+12+15, 0.2, Tri));
			break;	
		case 'l':
			add_item(&livenotes, (void*) create_note(12+12+17, 0.2, Tri));
			break;	
	}
}

*/


 return err;
/*error:
 Pa_Terminate();
 fprintf( stderr, "An error occured while using the portaudio stream\n" );
 fprintf( stderr, "Error number: %d\n", err );
 fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
 return err;
*/
}


int main(void) {
	printf("Hello world!\n");
	init_synth_audio();
	printf("All done\n");
	start_synth();
	
}
