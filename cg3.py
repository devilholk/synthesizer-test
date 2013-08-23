#encoding=utf-8
import tempfile, subprocess, os, imp
from shellstuff import cp, call
make_template = u'''
CC = gcc
CFLAGS = -Wall -O2 -march=native -std=gnu99 -fPIC -g
LDFLAGS =  -lpython2.7 -lm -shared
INCLUDES = -I/usr/include/python2.7 -Iinc
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

def compile_and_import(module):
	old_cwd = os.getcwdu()
	build_dir = tempfile.mkdtemp()
	try:
		write_file(build_dir, module.name+'.c', module.export())
		write_file(build_dir, 'Makefile', make_template%{'sources': module.name+'.c processor.o', 'lib': module.name+'.so'})
		cp('processor.o', build_dir)
		os.mkdir( os.path.join(build_dir,'inc') )
		cp('inc/processor.h', os.path.join(build_dir,'inc'))
		cp('inc/list.h', os.path.join(build_dir,'inc'))
		os.chdir(build_dir)
		subprocess.Popen( ("make",), stdout=subprocess.PIPE).communicate()
		os.chdir(old_cwd)
		imod = imp.load_dynamic(module.name, os.path.join(build_dir, module.name+'.so') )	
		return imod	
	except:
		raise
	finally:
		call( ['rm', '-r', build_dir] )
		

class Datatype():
	def __init__(self, name, to_python=None, from_python=None, ptr=False):
		self.name = name
		self.ptr=ptr
		if to_python:
			self.to_python = lambda var: to_python % var
			
		if from_python:
			self.from_python = lambda var: from_python % var
			
	def get_dt(self):
		return self.name+'*' if self.ptr else self.name
	
	def __repr__(self):
		return '<%s %s>'%(self.__class__.__name__, self.name) 

class Struct():
	def __init__(self, name):
		self.name = name
		self.members = []
	
	def init(self, name):

		members = ['.%s = %s' % (member.name, member.init() ) for member in self.members if member.default != None]
		
		res = '*%(name)s = (%(datatype)s) {\n\t%(members)s\n};\n' % {
			'name': name,
			'datatype': self.name,
			'members': ',\n\t'.join(members)
		}
		
		for member in self.members:
			if member.postinit:
				if hasattr(member.postinit, 'init'):
					res+=member.postinit.init('%s->%s'%(name, member.name), member)
				else:
					res+='%s->%s = %s;\n'%(name, member.name, member.postinit)
		
		return res
	

	def declare(self):

		members = ['%s %s;' % (member.datatype.get_dt(), member.name) for member in self.members]
		
		return 'typedef struct {\n\t%(members)s\n} %(datatype)s;' % {
			'datatype': self.name,
			'members': '\n\t'.join(members)
		}
	
class Var():
	def __init__(self, name, datatype, default=None, init=None):
		self.name = name
		self.datatype = datatype
		self.default = default	
		self.postinit=init

	def init(self):
		if hasattr(self.default,'init'):
			return self.default.init(self)
		else:
			return self.default
	
	def __repr__(self):
		return '<%s %s, %s, default=%s>'%(self.__class__.__name__, self.name, self.datatype, self.default) 


class Malloc():
	def __init__(self, size):
		self.size = size

	def __repr__(self):
		return '<%s %s>'%(self.__class__.__name__, self.size) 

	def init(self, name, member):
		return '%(name)s = (%(datatype)s*) malloc(sizeof(%(datatype)s) * %(size)s);\n' % {
			'name': name,
			'member': member.name,
			'datatype': member.datatype.name,
			'size': self.size
		}


class PythonExport():
	def __init__(self, name):
		self.objects = []
		self.name = name
		
	def export(self):
		class method():
			def __init__(self, name, desc):
				self.name=name
				self.desc = desc
		export=[]
		
		
		res=''
		
		res+='#include <Python.h>\n'		
		res+='#include <math.h>\n'		
		res+='#include "processor.h"\n'		
		
		for object in self.objects:
			res+=object.data.declare()			

		for object in self.objects:
			res+=object.process()	

		for object in self.objects:
			res+=object.constructor()			
			export.append(method('%s_create'%object.name, 'Create a new %s'%object.name))


		for object in self.objects:
			for prop in object.export:
				res+= Property.getter( object, object.data.members[prop] )
				res+= Property.setter( object, object.data.members[prop] )
				export.append(method('%s_get_%s'%(object.name, object.data.members[prop].name), 'Get %s of %s'%(object.data.members[prop].name, object.name)))
				export.append(method('%s_set_%s'%(object.name, object.data.members[prop].name), 'Set %s of %s'%(object.data.members[prop].name, object.name)))


			
		methods = ['{"%s",  %s, METH_VARARGS, "%s"}'%(method.name, method.name, method.desc) for method in export] +['{NULL, NULL, 0, NULL}']
		res+='''
			static PyMethodDef exported_methods[] = {
				%(methods)s
			};
	
			PyMODINIT_FUNC init%(name)s(void) {
			    Py_InitModule("%(name)s", exported_methods);
			}''' % {
				'name': self.name,
				'methods': ',\n\t'.join(methods)
			}
		return res

class PythonArgument():
	def __init__(self, index):
		self.index = index

	def init(self, var):
		return var.datatype.from_python('PyTuple_GetItem(args, %s)'%self.index)

	def __repr__(self):
		return '<%s %s>'%(self.__class__.__name__, self.index) 

class Object():
	def __init__(self, name):
		self.name = name
		self.properties = []
		self.on_init = None
		self.data= None
		self.processor=None

	def process(self):
		return '''int %(object)s_process (float* out, int sample_rate, int length, void* pdata) {
				%(object)s_data* data = (%(object)s_data*) pdata;
				%(process)s
		}\n''' % {
			'object': self.name,
			'process': self.processor
		}
	

	def constructor(self):

		def init():
			if self.on_init:
				return (
					'''%(object)s_data* data = (%(object)s_data*) malloc(sizeof(%(object)s_data));
						%(struct)s;
						%(init)s;
					''' % {
						'object': self.name,
						'init': self.on_init,
						'struct': self.data.init('data')
					})
			else:				#We should check if we needto init struct or not
				return (
					'''%(object)s_data* data = (%(object)s_data*) malloc(sizeof(%(object)s_data));
						%(struct)s;
					''' % {
						'object': self.name,
						'struct': self.data.init('data')
					})


		return (
			'''static PyObject *%(object)s_create (PyObject *self, PyObject *args) {

				%(init)s;
			
				Processor *processor = (Processor*) processor_create( %(object)s_process, %(data)s);
				return PyInt_FromLong((long) processor);
			}\n''' %
			{
				'object': self.name,
				'init': init(),
				'data': '(void*) data'
#				'data': '(void*) data' if self.on_init else 'NULL'			Check if struct exists
			})



class Property():
	@classmethod
	def getter(self, object, var):
		return (
			'''static PyObject* %(object)s_get_%(attribute)s (PyObject* self, PyObject* args) {
			
				Processor* processor = (Processor*) PyInt_AsLong( PyTuple_GetItem(args, 0) );	
				%(object)s_data* data = (%(object)s_data*) processor->data;
			
				return (%(data)s);
			}''' % 
		{
			'object': object.name, 
			'attribute': var.name,
			'data': var.datatype.to_python('data->' + var.name)
		})
		
	@classmethod
	def setter(self, object, var):
		return (
			'''static PyObject* %(object)s_set_%(attribute)s (PyObject* self, PyObject* args) {
			
				Processor* processor = (Processor*) PyInt_AsLong( PyTuple_GetItem(args, 0) );	
				%(object)s_data* data = (%(object)s_data*) processor->data;
				data->%(attribute)s = %(data)s;

				Py_RETURN_NONE;
			}''' % 
		{
			'object': object.name, 
			'attribute': var.name,
			'data': var.datatype.from_python('PyTuple_GetItem(args, 1)')
		})
		

datatypes = {
	'float': Datatype('float', 'PyFloat_FromDouble((double) %s)', '(float) PyFloat_AsDouble(%s)'),
	'int': Datatype('int', 'PyInt_FromLong((long) %s)', '(int) PyInt_AsLong(%s)'),
	'float*': Datatype('float',ptr=True),
}


