#encoding = utf-8

import tempfile, subprocess, os, imp

simple_eval_code = u'''

	#include <Python.h>
	#include <math.h>
	
	int test_func (float* out, int sample_rate, int length) {
	
		static float t=0.0;
	
		for (int i=0; i!=length;i++) {
			t+=1.0/sample_rate;
			*(out++) = %(left)s;
			*(out++) = %(right)s;
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

'''

simple_eval_make = u'''
CC = gcc
CFLAGS = -Wall -O2 -march=native -std=gnu99 -fPIC -g
LDFLAGS =  -lpython2.7 -lm -shared
INCLUDES = -I/usr/include/python2.7 -I../inc
SOURCES = %(sources)s
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = %(lib)s

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJECTS)
	rm $(EXECUTABLE)
'''


def write_file(path, name, data):
	f = open( os.path.join(path, name) , 'w')
	f.write(data.encode('utf-8'))
	f.close()
	

def build_function(code):
	build_dir = tempfile.mkdtemp()
	write_file(build_dir, 'a.c', simple_eval_code % code)
	write_file(build_dir, 'Makefile', simple_eval_make % {'sources': 'a.c', 'lib': 'a.so'})
	old_cwd = os.getcwdu()
	os.chdir(build_dir)
	subprocess.Popen( ("make",), stdout=subprocess.PIPE).communicate()
	os.chdir(old_cwd)
	return imp.load_dynamic('test', os.path.join(build_dir, 'a.so') ).get_pointer()