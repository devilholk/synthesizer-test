#encoding=utf-8
from cg3 import *


effect = Object('reverb')
effect.data = Struct('reverb_data')
effect.data.members.append(Var('buffersize', datatypes['int'], PythonArgument(0)))
effect.data.members.append(Var('length', datatypes['int'], PythonArgument(1)))
effect.data.members.append(Var('feedback', datatypes['float'], PythonArgument(2)))
effect.data.members.append(Var('buffer', datatypes['float*'], init=Malloc('data->buffersize')))
effect.data.members.append(Var('buffer_head', datatypes['int'], init='data->length % data->buffersize'))
effect.data.members.append(Var('buffer_tail', datatypes['int'], 0))

effect.on_init = '''
	//Clear buffer
	for (int i=0;i!=data->buffersize;i++) {
		data->buffer[i] = 0.0;
	}
'''


effect.processor = '''
		for (int i=0; i!=length;i++) {
		
			*(out++) = data->buffer[data->buffer_head] = *out + data->feedback * data->buffer[data->buffer_tail];
			data->buffer_tail++;
			if (data->buffer_tail > data->length) data->buffer_tail = 0;
			if (data->buffer_head > data->length) data->buffer_head = 0;
			data->buffer_head++;
		}
		return 1;	
'''

effect.export = (1,2)
module = PythonExport('testmodule')
module.objects.append(effect)

i = compile_and_import( module )
