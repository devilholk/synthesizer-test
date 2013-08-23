/*

	TODO
	
		When calling malloc, if fail, call PyErr_NoMemory	http://docs.python.org/2/c-api/exceptions.html#PyErr_NoMemory
		Install a clean up function to be called when stream is finished or python exited	Py_AtExit(func);

*/
#include <Python.h>
#include "debug.h"
#include "synthesizer.h"
//#include <math.h>
#include <stdlib.h>
#define SAMPLE_RATE 44100
//#include <unistd.h>
//#include <termios.h>

#include <portaudio.h>
#include "list.h"
#include "processor.h"
#include "channel.h"
#include "processor-test.h"

int start_synth(Processor*);
int stop_synth(void);
int destroy_synth_audio(void);
int init_synth_audio();

int running = 0;





/*static PyObject *synth_mixer_create(PyObject *self, PyObject *args) {	
	Mixer *mixer = mixer_create();
	return PyInt_FromLong((long)mixer);
}
*/
static PyObject *synth_channel_create(PyObject *self, PyObject *args) {	
	Channel *channel = channel_create();
	return PyInt_FromLong((long)channel);
}


static PyObject *synth_channel_create_processor(PyObject *self, PyObject *args) {	
	Channel *channel = (Channel*) PyInt_AsLong( PyTuple_GetItem(args, 0) );
	Processor *processor = processor_create( channel_processor, (void*) channel);
	return PyInt_FromLong((long) processor);
}




static PyObject *synth_channel_add_processor(PyObject *self, PyObject *args) {	
	Channel *channel = (Channel*) PyInt_AsLong( PyTuple_GetItem(args, 0) );
	Processor *processor = (Processor*) PyInt_AsLong( PyTuple_GetItem(args, 1) );
	ListItem *list_item = channel_add_processor(channel, processor);
	return PyInt_FromLong((long)list_item);
}


static PyObject *synth_channel_del_processor(PyObject *self, PyObject *args) {	
	Channel *channel = (Channel*) PyInt_AsLong( PyTuple_GetItem(args, 0) );
	ListItem *list_item = (ListItem*) PyInt_AsLong( PyTuple_GetItem(args, 1) );
	channel_remove_processor(channel, list_item);
	Py_RETURN_NONE;
}

static PyObject *synth_channel_add_note(PyObject *self, PyObject *args) {	
	Channel *channel = (Channel*) PyInt_AsLong( PyTuple_GetItem(args, 0) );
	Processor *note = (Processor*) PyInt_AsLong( PyTuple_GetItem(args, 1) );
	ListItem *list_item = channel_add_note(channel, note);
	return PyInt_FromLong((long)list_item);
}


static PyObject *synth_channel_del_note(PyObject *self, PyObject *args) {	
	Channel *channel = (Channel*) PyInt_AsLong( PyTuple_GetItem(args, 0) );
	ListItem *list_item = (ListItem*) PyInt_AsLong( PyTuple_GetItem(args, 1) );
	channel_remove_note(channel, list_item);
	Py_RETURN_NONE;
}


/*

static PyObject *synth_mixer_destroy(PyObject *self, PyObject *args) {
	Mixer *mixer = (Mixer*) PyInt_AsLong( PyTuple_GetItem(args, 0) );
	mixer_destroy(mixer);
	Py_RETURN_NONE;
}
*/
static PyObject *synth_channel_destroy(PyObject *self, PyObject *args) {
	Channel *channel = (Channel*) PyInt_AsLong( PyTuple_GetItem(args, 0) );
	channel_destroy(channel);
	Py_RETURN_NONE;
}

static PyObject *synth_start(PyObject *self, PyObject *args) {
	
	Processor *synth = (Processor*) PyInt_AsLong( PyTuple_GetItem(args, 0) );
	
	return PyInt_FromLong(init_synth_audio() || start_synth(synth));
}

static PyObject *synth_channel_set(PyObject *self, PyObject *args) {
	
	Channel *channel = (Channel*) PyInt_AsLong( PyTuple_GetItem(args, 0) );
	channel->volume = (float) PyFloat_AsDouble( PyTuple_GetItem(args, 1) );
	channel->flags = (int) PyInt_AsLong( PyTuple_GetItem(args, 2) );
	
	Py_RETURN_NONE;
}

static PyObject *synth_channel_get(PyObject *self, PyObject *args) {
	
	Channel *channel = (Channel*) PyInt_AsLong( PyTuple_GetItem(args, 0) );
	
	return PyTuple_Pack(2, 
		PyFloat_FromDouble( (double) channel->volume),
		PyInt_FromLong( (long) channel->flags) 
	);
}




static PyObject *synth_processor_test_generic_sine(PyObject *self, PyObject *args) {

	Processor *processor = testproc_generic_sine(
		(float) PyFloat_AsDouble( PyTuple_GetItem(args, 0)),
		(float) PyFloat_AsDouble( PyTuple_GetItem(args, 1))
	);
	
	return PyInt_FromLong( (long) processor);
}


static PyObject *synth_processor_test_generic_sine_get(PyObject *self, PyObject *args) {

	Processor *processor = (Processor*) PyInt_AsLong( PyTuple_GetItem(args, 0));

	float* data = (float*) processor->data;


	return PyTuple_Pack(2, 
		PyFloat_FromDouble( (double) data[0]),
		PyFloat_FromDouble( (double) data[1])
	);
}

static PyObject *synth_processor_test_generic_sine_set(PyObject *self, PyObject *args) {

	Processor *processor = (Processor*) PyInt_AsLong( PyTuple_GetItem(args, 0));

	float* data = (float*) processor->data;
	data[0] = (float) PyFloat_AsDouble( PyTuple_GetItem(args, 1));
	data[1] = (float) PyFloat_AsDouble( PyTuple_GetItem(args, 2));

	Py_RETURN_NONE;
}



static PyObject *synth_processor_test_get_sine440_low(PyObject *self, PyObject *args) {

	Processor *processor = processor_create( testproc_sine440_low, NULL );
	
	return PyInt_FromLong( (long) processor);
}


static PyObject *synth_processor_destroy(PyObject *self, PyObject *args) {

	Processor *processor = (Processor*) PyInt_AsLong( PyTuple_GetItem(args, 0) );
	processor_destroy( processor );
	Py_RETURN_NONE;
}


static PyObject *synth_processor_test_filter(PyObject *self, PyObject *args) {
//Processor* testproc_filter (float start, float stop, int tapbits, int sample_rate) {

	DEBUG_PRINT ( "synth_processor_test_filter argument 1: %f",(float) PyFloat_AsDouble( PyTuple_GetItem(args, 0) ));

	Processor *filter = testproc_filter(
		(float) PyFloat_AsDouble( PyTuple_GetItem(args, 0) ),
		(float) PyFloat_AsDouble( PyTuple_GetItem(args, 1) ),
		(int) PyInt_AsLong( PyTuple_GetItem(args, 2) ),
		SAMPLE_RATE
	);

	return PyInt_FromLong( (long) filter);

}





#ifdef DEBUG

/*	mess with this later
	static PyObject *synth_channel_debug_dump(PyObject *self, PyObject *args) {
	
		Channel *channel = (Channel*) PyInt_AsLong( PyTuple_GetItem(args, 0) );

		DEBUG_CHANNEL_DUMP (channel);

		Py_RETURN_NONE;
	}
*/


	static PyObject *synth_channel_debug(PyObject *self, PyObject *args) {
	
		Channel *channel = (Channel*) PyInt_AsLong( PyTuple_GetItem(args, 0) );

		return PyTuple_Pack(5, 
			PyInt_FromLong( (long) &channel->notes),
			PyInt_FromLong( (long) &channel->processors),
			PyInt_FromLong( (long) channel->buffer),
			PyFloat_FromDouble( (double) channel->volume),
			PyInt_FromLong( (long) channel->flags)
		);

	}

	static PyObject *synth_list_debug(PyObject *self, PyObject *args) {
	
		List *list = (List*) PyInt_AsLong( PyTuple_GetItem(args, 0) );

		return PyTuple_Pack(2, 
			PyInt_FromLong( (long) list->first),
			PyInt_FromLong( (long) list->last)
		);

	}

	static PyObject *synth_listitem_debug(PyObject *self, PyObject *args) {
	
		ListItem *list_item = (ListItem*) PyInt_AsLong( PyTuple_GetItem(args, 0) );

		return PyTuple_Pack(3, 
			PyInt_FromLong( (long) list_item->item),
			PyInt_FromLong( (long) list_item->prev),
			PyInt_FromLong( (long) list_item->next)
		);

	}



	static PyObject *synth_processor_debug(PyObject *self, PyObject *args) {
	
		Processor *processor = (Processor*) PyInt_AsLong( PyTuple_GetItem(args, 0) );

		return PyTuple_Pack(2, 
			PyInt_FromLong( (long) processor->function),
			PyInt_FromLong( (long) processor->data)
		);

	}



#endif 

static PyMethodDef SynthMethods[] = {
    {"init",  synth_start, METH_VARARGS, "Setup audio and start synthesizer"},
/*
    {"mixer_create",  synth_mixer_create, METH_VARARGS, "Create mixer"},
    {"mixer_destroy",  synth_mixer_destroy, METH_VARARGS, "Destroy mixer"},
*/
    {"channel_create",  synth_channel_create, METH_VARARGS, "Create channel"},
    {"channel_destroy",  synth_channel_destroy, METH_VARARGS, "Destroy channel"},

    {"channel_set",  synth_channel_set, METH_VARARGS, "Set data of channel"},
    {"channel_get",  synth_channel_get, METH_VARARGS, "Get data of channel"},

    {"channel_add_note",  synth_channel_add_note, METH_VARARGS, "Add finite subprocessor to channel"},
    {"channel_del_note",  synth_channel_del_note, METH_VARARGS, "Remove finite subprocessor from channel"},

    {"channel_add_processor",  synth_channel_add_processor, METH_VARARGS, "Add post processor to channel"},
    {"channel_del_processor",  synth_channel_del_processor, METH_VARARGS, "Remove post processor from channel"},
	
	 {"channel_create_processor", synth_channel_create_processor, METH_VARARGS, "Create new instance of channel processor"},

    {"processor_test_get_sine440_low",  synth_processor_test_get_sine440_low, METH_VARARGS, "Create a 20% amplitude 440 Hz testing processor"},

    {"processor_test_generic_sine",  synth_processor_test_generic_sine, METH_VARARGS, "Create generic sine generator"},
    {"processor_test_generic_sine_set",  synth_processor_test_generic_sine_set, METH_VARARGS, "Set data to generic sine generator"},
    {"processor_test_generic_sine_get",  synth_processor_test_generic_sine_get, METH_VARARGS, "Get data from generic sine generator"},


    {"processor_test_filter",  synth_processor_test_filter, METH_VARARGS, "Filter test stuff"},


    {"processor_destroy",  synth_processor_destroy, METH_VARARGS, "Destroy processor"},

#ifdef DEBUG		//TODO: mess with this later

	 {"list_debug",  synth_list_debug, METH_VARARGS, "Get debug data of list"},
	 {"listitem_debug",  synth_listitem_debug, METH_VARARGS, "Get debug data of listitem"},
	 {"processor_debug",  synth_processor_debug, METH_VARARGS, "Get debug data of processor"},
	 {"channel_debug",  synth_channel_debug, METH_VARARGS, "Get debug data of channel"},


//	    {"channel_dump",  synth_channel_debug_dump, METH_VARARGS, "Show debug data of channel"},

#endif

/*
    {"mixer_add_channel",  synth_mixer_add_channel, METH_VARARGS, "Add channel to mixer"},
    {"mixer_add_processor",  synth_mixer_add_processor, METH_VARARGS, "Add processor to mixer"},
    {"mixer_remove_channel",  synth_mixer_remove_channel, METH_VARARGS, "Remove channel from mixer"},
    {"mixer_remove_processor",  synth_mixer_remove_processor, METH_VARARGS, "Remove processor from mixer"},
*/
    {NULL, NULL, 0, NULL}        /* Sentinel */
};



PyMODINIT_FUNC initsynth(void) {
    Py_InitModule("synth", SynthMethods);
}


static int liveplayCallback( const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData ) {

	float *out = (float*)outputBuffer;
	
	(void) timeInfo; /* Prevent unused variable warnings. */
	(void) statusFlags;
	(void) inputBuffer;

	if (userData) {
		Processor *proc = (Processor*) userData;
		int result = proc->function(out, SAMPLE_RATE, framesPerBuffer, proc->data);
		return result ? paContinue : paComplete;
	}
	
	return paComplete;
}
	

static void StreamFinished( void* userData )
{
}

PaStreamParameters outputParameters;
PaStream *stream;


int init_synth_audio(void) {
	PaError err;
	err = Pa_Initialize();
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

 	return 0;

}

int start_synth(Processor *synthesizerCallback) {

	outputParameters.device = Pa_GetDefaultOutputDevice(); // default output device 
	if (outputParameters.device == paNoDevice) {
		return -1;
	}

	

	outputParameters.channelCount = 1;  
	outputParameters.sampleFormat = paFloat32;  
	outputParameters.suggestedLatency = 0.01;
	outputParameters.hostApiSpecificStreamInfo = NULL;
	PaError err;
		
	err = Pa_OpenStream(
		&stream,
		NULL, // no input 
		&outputParameters,
		SAMPLE_RATE,
		FRAMES_PER_BUFFER,
		paNoFlag, 
		liveplayCallback,
		synthesizerCallback	//userdata
	);
	if( err != paNoError ) return err;
	
	err = Pa_SetStreamFinishedCallback( stream, &StreamFinished );
	if( err != paNoError ) return err;
	
	err = Pa_StartStream( stream );
	if( err != paNoError ) return err;
	
	return err;
}

