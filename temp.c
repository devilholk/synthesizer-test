#include <Python.h>
#include <math.h>
#include "processor.h"
typedef struct {
	float frequency;
	float amplitude;
	float time;
	int cont;
} sine_data;int sine_process (float* out, int sample_rate, int length, void* pdata) {
				sine_data* data = (sine_data*) pdata;
				
		for (int i=0; i!=length;i++) {
			*(out++) = sin(data->time) * data->amplitude;
			data->time = fmod(data->time + 2.0 * M_PI * data->frequency / (float)sample_rate, 2.0 * M_PI);
		}
		return (data->cont);

		}
static PyObject *sine_create (PyObject *self, PyObject *args) {

				sine_data* data = (sine_data*) malloc(sizeof(sine_data));
						*data = (sine_data) {
	.frequency = (float) PyFloat_AsDouble(PyTuple_GetItem(args, 0)),
	.amplitude = (float) PyFloat_AsDouble(PyTuple_GetItem(args, 1)),
	.time = 0.0,
	.cont = 1
};
;
					;
			
				Processor *processor = (Processor*) processor_create( sine_process, (void*) data);
				return PyInt_FromLong((long) processor);
			}
static PyObject* sine_get_frequency (PyObject* self, PyObject* args) {
			
				Processor* processor = (Processor*) PyInt_AsLong( PyTuple_GetItem(args, 0) );	
				sine_data* data = (sine_data*) processor->data;
			
				return (PyFloat_FromDouble((double) data->frequency));
			}static PyObject* sine_set_frequency (PyObject* self, PyObject* args) {
			
				Processor* processor = (Processor*) PyInt_AsLong( PyTuple_GetItem(args, 0) );	
				sine_data* data = (sine_data*) processor->data;
				data->frequency = (float) PyFloat_AsDouble(PyTuple_GetItem(args, 1));

				Py_RETURN_NONE;
			}static PyObject* sine_get_amplitude (PyObject* self, PyObject* args) {
			
				Processor* processor = (Processor*) PyInt_AsLong( PyTuple_GetItem(args, 0) );	
				sine_data* data = (sine_data*) processor->data;
			
				return (PyFloat_FromDouble((double) data->amplitude));
			}static PyObject* sine_set_amplitude (PyObject* self, PyObject* args) {
			
				Processor* processor = (Processor*) PyInt_AsLong( PyTuple_GetItem(args, 0) );	
				sine_data* data = (sine_data*) processor->data;
				data->amplitude = (float) PyFloat_AsDouble(PyTuple_GetItem(args, 1));

				Py_RETURN_NONE;
			}static PyObject* sine_get_cont (PyObject* self, PyObject* args) {
			
				Processor* processor = (Processor*) PyInt_AsLong( PyTuple_GetItem(args, 0) );	
				sine_data* data = (sine_data*) processor->data;
			
				return (PyInt_FromLong((long) data->cont));
			}static PyObject* sine_set_cont (PyObject* self, PyObject* args) {
			
				Processor* processor = (Processor*) PyInt_AsLong( PyTuple_GetItem(args, 0) );	
				sine_data* data = (sine_data*) processor->data;
				data->cont = (int) PyInt_AsLong(PyTuple_GetItem(args, 1));

				Py_RETURN_NONE;
			}
			static PyMethodDef exported_methods[] = {
				{"sine_create",  sine_create, METH_VARARGS, "Create a new sine"},
	{"sine_get_frequency",  sine_get_frequency, METH_VARARGS, "Get frequency of sine"},
	{"sine_set_frequency",  sine_set_frequency, METH_VARARGS, "Set frequency of sine"},
	{"sine_get_amplitude",  sine_get_amplitude, METH_VARARGS, "Get amplitude of sine"},
	{"sine_set_amplitude",  sine_set_amplitude, METH_VARARGS, "Set amplitude of sine"},
	{"sine_get_cont",  sine_get_cont, METH_VARARGS, "Get cont of sine"},
	{"sine_set_cont",  sine_set_cont, METH_VARARGS, "Set cont of sine"},
	{NULL, NULL, 0, NULL}
			};
	
			PyMODINIT_FUNC initsine(void) {
			    Py_InitModule("sine", exported_methods);
			}