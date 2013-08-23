#encoding=utf-8

import tempfile, subprocess, os, imp

parameter_code_c = u'''
static PyObject *%(object)s_get_%(parameter)s (PyObject *self, PyObject *args) {

	Processor *processor = (Processor*) PyInt_AsLong( PyTuple_GetItem(args, 0) );	
	%(object)s_data* data = (%(object)s_data*) processor->data;

	return (%(to_python)s data->%(parameter)s));
}

static PyObject *%(object)s_set_%(parameter)s (PyObject *self, PyObject *args) {

	Processor *processor = (Processor*) PyInt_AsLong( PyTuple_GetItem(args, 0) );	

	%(object)s_data* data = (%(object)s_data*) processor->data;
	data->%(parameter)s = %(from_python)s(PyTuple_GetItem(args, 1));

	Py_RETURN_NONE;
}
'''

module_code_c = u'''
%(includes)s

typedef struct {
	%(struct_members)s
} %(object)s_data;


int %(object)s_process (float* out, int sample_rate, int length, void *dataptr) {

	%(object)s_data* data = (%(object)s_data*) dataptr;

	%(process_function)s
}

static PyObject *%(object)s_create (PyObject *self, PyObject *args) {

	%(object)s_data* data = (%(object)s_data*) malloc(sizeof(%(object)s_data));

	*data = (%(object)s_data) {
		%(struct_create_init)s
	};

	%(init_function)s;

	Processor *processor = (Processor*) processor_create( %(object)s_process, data);

	return PyInt_FromLong((long) processor);

}

static PyObject *%(object)s_destroy (PyObject *self, PyObject *args) {

	Processor *processor = (Processor*) PyInt_AsLong( PyTuple_GetItem(args, 0) );	

	%(destroy_function)s;

	free(processor->data);
	free(processor);
	Py_RETURN_NONE;
}

%(parameters)s

static PyMethodDef PythonMethods[] = {%(python_methods)s};

PyMODINIT_FUNC init%(module)s(void) {
    Py_InitModule("%(module)s", PythonMethods);
}

'''

include_code_c = u'#include %(header)s'
struct_member_code_c = u'%(m_type)s %(m_name)s;'
struct_member_init_c = u'.%(m_name)s = %(from_python)s( PyTuple_GetItem(args, %(argpos)s) )'


include_headers = ['<Python.h>', '<math.h>', '"debug.h"', '"processor.h"']

typemapping = {
	'float': {'from_py': '(float) PyFloat_AsDouble', 'to_py': 'PyFloat_FromDouble((float)'},
	'int': {'from_py': '(int) PyInt_AsLong', 'to_py': 'PyInt_FromLong((long)'}
}

all_members = [
	{'m_type': 'float', 'm_name': 'feedback'},
	{'m_type': 'int', 'm_name': 'bufsize'},
	{'m_type': 'int', 'm_name': 'length'},
	{'m_type': 'float*', 'm_name': 'buffer'}
]

init_members = [0, 1, 2]

exported_members = [0, 2]
name='reverb'

methods=[]
struct_init = []
argpos = 0
for member_id in init_members:
	data = dict(all_members[member_id])
	data['from_python'] = typemapping[data['m_type']]['from_py']
	data['argpos']=argpos
	struct_init.append( struct_member_init_c % data )
	argpos+=1

parameters = []
for member_id in exported_members:
	data = all_members[member_id]
	parameters.append( parameter_code_c % {
		'object': name,
		'parameter': data['m_name'],
		'to_python': typemapping[data['m_type']]['to_py'],
		'from_python': typemapping[data['m_type']]['from_py']
	})


c_code = module_code_c % {
	'object': 'reverb',
	'includes': '\n'.join([include_code_c % {'header': header} for header in include_headers]),
	'struct_members': '\n\t'.join([struct_member_code_c % member for member in all_members]),
	'process_function': 'DEBUG_PRINT( "Processing data!" );\nreturn 0;',
	'struct_create_init':  ',\n\t\t'.join( struct_init ),
	'init_function': 'DEBUG_PRINT ("Initializing function")',
	'destroy_function': 'DEBUG_PRINT ("Destroy function")',
	'parameters': u''.join(parameters),
	'python_methods': u'\n\t%s\n'% '\n\t'.join(methods),
	'module': 'testmodul',
}

simple_make = u'''
CC = gcc
CFLAGS = -Wall -O2 -march=native -std=gnu99 -fPIC -g
LDFLAGS =  -lpython2.7 -lm -shared
INCLUDES = -I/usr/include/python2.7 -I%(cwd)s/inc
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


makefile = simple_make % {'sources': 'a.c', 'lib': 'a.so', 'cwd': os.getcwd()}
print c_code

def write_file(path, name, data):
	f = open( os.path.join(path, name) , 'w')
	f.write(data.encode('utf-8'))
	f.close()
	

def build_function():
	build_dir = tempfile.mkdtemp()
	write_file(build_dir, 'a.c', c_code)
	write_file(build_dir, 'Makefile', makefile)
	old_cwd = os.getcwdu()
	os.chdir(build_dir)
	subprocess.Popen( ("make",), stdout=subprocess.PIPE).communicate()
	os.chdir(old_cwd)
	return imp.load_dynamic('test', os.path.join(build_dir, 'a.so') ).get_pointer()



print build_function()
