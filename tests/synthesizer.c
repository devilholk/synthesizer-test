#include <Python.h>
#include <math.h>

int test_func (float* out, int sample_rate, int length) {

	static float ttime=0.0;

	for (int i=0; i!=length;i++) {

		ttime+=440.0*2.0*M_PI/sample_rate;
		*(out++) = sin(ttime)*0.2;
		*(out++) = sin(ttime)*0.2;
	}
	return 1;	
}

static PyObject *get_pointer(PyObject *self, PyObject *args) {
	return PyInt_FromLong((long) &test_func);
}

static PyMethodDef SynthMethods[] = {
    {"get_pointer",  get_pointer, METH_VARARGS, "Get instrument pointer"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC inittest(void) {
    Py_InitModule("test", SynthMethods);
}

