#include <Python.h>
#include <math.h>
#include "processor.h"
typedef struct {
	int buffersize;
	int length;
	float feedback;
	float* buffer;
	int buffer_head;
	int buffer_tail;
} reverb_data;int reverb_process (float* out, int sample_rate, int length, void* pdata) {
				reverb_data* data = (reverb_data*) pdata;
				
		for (int i=0; i!=length;i++) {
			*(out++) = 0.0;
		}
		return 1;	

		}
static PyObject *reverb_create (PyObject *self, PyObject *args) {

				reverb_data* data = (reverb_data*) malloc(sizeof(reverb_data));
						*data = (reverb_data) {
	.buffersize = (int) PyInt_AsLong(PyTuple_GetItem(args, 0)),
	.length = (int) PyInt_AsLong(PyTuple_GetItem(args, 1)),
	.feedback = (float) PyFloat_AsDouble(PyTuple_GetItem(args, 2)),
	.buffer_tail = 0
};
data->buffer = (float*) malloc(sizeof(float) * data->buffersize);
data->buffer_head = data->length % data->buffersize;
;
						
	//Clear buffer
	for (int i=0;i!=data->buffersize;i++) {
		data->buffer[i] = 0.0;
	}
;
					;
			
				Processor *processor = (Processor*) processor_create( reverb_process, (void*) data);
				return PyInt_FromLong((long) processor);
			}
static PyObject* reverb_get_length (PyObject* self, PyObject* args) {
			
				Processor* processor = (Processor*) PyInt_AsLong( PyTuple_GetItem(args, 0) );	
				reverb_data* data = (reverb_data*) processor->data;
			
				return (PyInt_FromLong((long) data->length));
			}static PyObject* reverb_set_length (PyObject* self, PyObject* args) {
			
				Processor* processor = (Processor*) PyInt_AsLong( PyTuple_GetItem(args, 0) );	
				reverb_data* data = (reverb_data*) processor->data;
				data->length = (int) PyInt_AsLong(PyTuple_GetItem(args, 1));

				Py_RETURN_NONE;
			}static PyObject* reverb_get_feedback (PyObject* self, PyObject* args) {
			
				Processor* processor = (Processor*) PyInt_AsLong( PyTuple_GetItem(args, 0) );	
				reverb_data* data = (reverb_data*) processor->data;
			
				return (PyFloat_FromDouble((double) data->feedback));
			}static PyObject* reverb_set_feedback (PyObject* self, PyObject* args) {
			
				Processor* processor = (Processor*) PyInt_AsLong( PyTuple_GetItem(args, 0) );	
				reverb_data* data = (reverb_data*) processor->data;
				data->feedback = (float) PyFloat_AsDouble(PyTuple_GetItem(args, 1));

				Py_RETURN_NONE;
			}
			static PyMethodDef exported_methods[] = {
				{"reverb_create",  reverb_create, METH_VARARGS, "Create a new reverb"},
	{"reverb_get_length",  reverb_get_length, METH_VARARGS, "Get length of reverb"},
	{"reverb_set_length",  reverb_set_length, METH_VARARGS, "Set length of reverb"},
	{"reverb_get_feedback",  reverb_get_feedback, METH_VARARGS, "Get feedback of reverb"},
	{"reverb_set_feedback",  reverb_set_feedback, METH_VARARGS, "Set feedback of reverb"},
	{NULL, NULL, 0, NULL}
			};
	
			PyMODINIT_FUNC inittestmodule(void) {
			    Py_InitModule("testmodule", exported_methods);
			}
