/*

	TODO
	
		When calling malloc, if fail, call PyErr_NoMemory	http://docs.python.org/2/c-api/exceptions.html#PyErr_NoMemory
		Install a clean up function to be called when stream is finished or python exited	Py_AtExit(func);

*/

#include "synthesizer.h"
//#include <math.h>
#include <Python.h>
#include <stdlib.h>
#define SAMPLE_RATE 44100
//#include <unistd.h>
//#include <termios.h>

#include <portaudio.h>
#include "list.h"
#include "mixer.h"
#include "processor.h"

int start_synth(Processor*);
int stop_synth(void);
int destroy_synth_audio(void);
int init_synth_audio();

int running = 0;



static PyObject *synth_mixer_create(PyObject *self, PyObject *args) {	
	Mixer *mixer = mixer_create();
	return PyInt_FromLong((long)mixer);
}

static PyObject *synth_channel_create(PyObject *self, PyObject *args) {	
	Channel *channel = channel_create();
	return PyInt_FromLong((long)channel);
}

static PyObject *synth_mixer_add_channel(PyObject *self, PyObject *args) {	
	Mixer *mixer = (Mixer*) PyInt_AsLong( PyTuple_GetItem(args, 0) );
	Channel *channel = (Channel*) PyInt_AsLong( PyTuple_GetItem(args, 1) );
	ListItem *list_item = mixer_add_channel(mixer, channel);
	return PyInt_FromLong((long)list_item);
}

static PyObject *synth_mixer_add_processor(PyObject *self, PyObject *args) {	
	Mixer *mixer = (Mixer*) PyInt_AsLong( PyTuple_GetItem(args, 0) );
	Processor *processor = (Processor*) PyInt_AsLong( PyTuple_GetItem(args, 1) );
	ListItem *list_item = mixer_add_processor(mixer, processor);
	return PyInt_FromLong((long)list_item);
}

static PyObject *synth_mixer_remove_channel(PyObject *self, PyObject *args) {	
	Mixer *mixer = (Mixer*) PyInt_AsLong( PyTuple_GetItem(args, 0) );
	ListItem *list_item = (ListItem*) PyInt_AsLong( PyTuple_GetItem(args, 1) );
	mixer_remove_channel(mixer, list_item);
	Py_RETURN_NONE;
}

static PyObject *synth_mixer_remove_processor(PyObject *self, PyObject *args) {	
	Mixer *mixer = (Mixer*) PyInt_AsLong( PyTuple_GetItem(args, 0) );
	ListItem *list_item = (ListItem*) PyInt_AsLong( PyTuple_GetItem(args, 1) );
	mixer_remove_processor(mixer, list_item);
	Py_RETURN_NONE;
}


static PyObject *synth_mixer_destroy(PyObject *self, PyObject *args) {
	Mixer *mixer = (Mixer*) PyInt_AsLong( PyTuple_GetItem(args, 0) );
	mixer_destroy(mixer);
	Py_RETURN_NONE;
}

static PyObject *synth_channel_destroy(PyObject *self, PyObject *args) {
	Channel *channel = (Channel*) PyInt_AsLong( PyTuple_GetItem(args, 0) );
	channel_destroy(channel);
	Py_RETURN_NONE;
}

static PyObject *synth_start(PyObject *self, PyObject *args) {
	
	Processor *synth = (Processor*) PyInt_AsLong( PyTuple_GetItem(args, 0) );
	
	return PyInt_FromLong(init_synth_audio() || start_synth(synth));
}


static PyMethodDef SynthMethods[] = {
    {"init",  synth_start, METH_VARARGS, "Setup audio and start synthesizer"},
    {"mixer_create",  synth_mixer_create, METH_VARARGS, "Create mixer"},
    {"mixer_destroy",  synth_mixer_destroy, METH_VARARGS, "Destroy mixer"},
    {"channel_create",  synth_channel_create, METH_VARARGS, "Create channel"},
    {"channel_destroy",  synth_channel_destroy, METH_VARARGS, "Destroy channel"},
    {"mixer_add_channel",  synth_mixer_add_channel, METH_VARARGS, "Add channel to mixer"},
    {"mixer_add_processor",  synth_mixer_add_processor, METH_VARARGS, "Add processor to mixer"},
    {"mixer_remove_channel",  synth_mixer_remove_channel, METH_VARARGS, "Remove channel from mixer"},
    {"mixer_remove_processor",  synth_mixer_remove_processor, METH_VARARGS, "Remove processor from mixer"},

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

