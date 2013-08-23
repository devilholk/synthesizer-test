#encoding=utf-8
from cg3 import *


instrument = Object('sine')
instrument.data = Struct('sine_data')
instrument.data.members.append(Var('frequency', datatypes['float'], PythonArgument(0)))
instrument.data.members.append(Var('amplitude', datatypes['float'], PythonArgument(1)))
instrument.data.members.append(Var('time', datatypes['float'], 0.0))


instrument.data.members.append(Var('cont', datatypes['int'], 1))
instrument.data.members.append(Var('value', datatypes['float'], 0.0))
instrument.data.members.append(Var('time2', datatypes['float'], 0.0))
instrument.data.members.append(Var('amplitude2', datatypes['float'], 1.0))

instrument.data.members.append(Var('start_frequency', datatypes['float'], 0.0))
instrument.data.members.append(Var('target_frequency', datatypes['float'], 0.0))
instrument.data.members.append(Var('start_amplitude', datatypes['float'], 0.0))
instrument.data.members.append(Var('target_amplitude', datatypes['float'], 0.0))
instrument.data.members.append(Var('current_sweep_time', datatypes['float'], 100.0))
instrument.data.members.append(Var('total_sweep_time', datatypes['float'], 0.1))

instrument.processor = '''
		for (int i=0; i!=length;i++) {
			*(out++) += data->value;
			data->time+=data->frequency*8.0 / (float)sample_rate;
			if (data->time > 1.0) {
				data->time = fmod(data->time, 1.0);
				data->value = (((float)rand()/(float)RAND_MAX)*2.0-1.0) * data->amplitude;
			}
		}
		return (data->cont);
'''

instrument.processor = '''
		for (int i=0; i!=length;i++) {
			*(out++) += (data->value + data->time2)/2.0;
			data->time+=data->frequency * 8.0 / (float)sample_rate;
			data->time2= fmod(data->time2 + data->frequency / (float)sample_rate, 1.0);
			if (data->time > 1.0) {
				data->time = fmod(data->time, 1.0);
				data->value = (((float)rand()/(float)RAND_MAX)*2.0-1.0) * data->amplitude;
			}
		}
		return (data->cont);
'''

instrument.processor = '''
		for (int i=0; i!=length;i++) {
			*(out++) += data->amplitude2*(data->value + data->time2);
			data->time+=data->frequency * 8.0 / (float)sample_rate;
			data->time2= fmod(data->time2 + data->frequency / (float)sample_rate, 1.0);
			if (data->time > 1.0) {
				data->time = fmod(data->time, 1.0);
				data->value = (((float)rand()/(float)RAND_MAX)*2.0-1.0) * data->amplitude;
			}
			data->amplitude2=data->amplitude2 * j0(data->amplitude2*0.1);-
			if (data->amplitude2 < 0.001) {
				data->cont= 0;
			}
		}
		return (data->cont);
'''



instrument.processor = '''
		for (int i=0; i!=length;i++) {
			float timestep = data->frequency / (float)sample_rate;
			*(out++) += (fmod(data->time, 2.0)-1.0) * data->amplitude;
			data->time+=timestep;
			if (data->time > 1.0) {
				data->time = fmod(data->time, 1.0);
			}
			
			if (data->current_sweep_time < data->total_sweep_time) {
				data->frequency = (data->target_frequency - data->start_frequency) * data->current_sweep_time / data->total_sweep_time + data->start_frequency;
				data->amplitude = (data->target_amplitude - data->start_amplitude) * data->current_sweep_time / data->total_sweep_time + data->start_amplitude;
				data->current_sweep_time += 1.0 / (float)sample_rate;
			}
		}
		return (data->cont);
'''


instrument.processor = '''
		for (int i=0; i!=length;i++) {
			float timestep = (data->frequency *(1+ data->time2 * 0.25) ) / (float)sample_rate;
	
			*(out++) += (data->time > 0.5) ? data->amplitude : 0.0;
			data->time+=timestep;
			if (data->time > 1.0) {
				data->time = fmod(data->time, 1.0);
			}

			data->time2 += (420.0) / (float)sample_rate;
 			if (data->time2 > 1.0) {
				data->time2 = fmod(data->time2, 1.0);
			}

			
			if (data->current_sweep_time < data->total_sweep_time) {
				data->frequency = (data->target_frequency - data->start_frequency) * data->current_sweep_time / data->total_sweep_time + data->start_frequency;
				data->amplitude = (data->target_amplitude - data->start_amplitude) * data->current_sweep_time / data->total_sweep_time + data->start_amplitude;
				data->current_sweep_time += 1.0 / (float)sample_rate;
			}
		}
		return (data->cont);
'''



instrument.export = (0,1,3,7,8,9,10,11,12)
module = PythonExport('sine')
module.objects.append(instrument)

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
			float* in = out;
			*(out++) += data->feedback * data->buffer[data->buffer_tail];
			data->buffer[data->buffer_head] = *in;
			
			data->buffer_tail++;
			data->buffer_head++;
			if (data->buffer_tail > data->length) data->buffer_tail = 0;
			if (data->buffer_head > data->length) data->buffer_head = 0;
		}
		return 1;	
'''

effect.export = (1,2)

module.objects.append(effect)



sinemod = compile_and_import( module )


from synthesizer import *

import synth
channel = Channel()
channel.flags = 1
channel.volume = 0.2
start( channel )

import midi
midi.Capture
import math
#i = sinemod.sine_create( 440.0, 0.25 )

rev = sinemod.reverb_create(3000, 3000, 0.4)

synth.channel_add_processor( channel._ref, rev )


notes = {}
monophonic_note = None
trackchan = 0
transpose = 0
keycount = 0
last_frequency = None
last_amplitude = None
notelist={}
def handler( cmd ):
	global keycount, monophonic_note, last_frequency,last_amplitude, notelist



	if isinstance(cmd, midi.Command.Note_ON):
		if cmd.chan == trackchan or True:
			cmd.note+=-24+3+transpose

			if not cmd.note in notelist:
				new_frequency = 27.5*math.pow(2.0, cmd.note /12.0)
				new_amplitude = cmd.vel/300.0
				note = sinemod.sine_create( new_frequency, new_amplitude )
				notelist[cmd.note] =	note			
				synth.channel_add_note( channel._ref, 	note )

#			if cmd.vel != 0:
#				keycount += 1
#				new_frequency = 27.5*math.pow(2.0, cmd.note /12.0)
#				new_amplitude = cmd.vel/300.0
#				if keycount == 1:
#					monophonic_note = sinemod.sine_create( new_frequency, new_amplitude )
#					last_frequency = new_frequency				
#					last_amplitude = new_amplitude
#					synth.channel_add_note( channel._ref, 	monophonic_note)
#				else:
#					sinemod.sine_set_target_frequency(monophonic_note, new_frequency)
#					sinemod.sine_set_start_frequency(monophonic_note, last_frequency)
#					sinemod.sine_set_target_amplitude(monophonic_note, new_amplitude)
#					sinemod.sine_set_start_amplitude(monophonic_note, last_amplitude)
#					sinemod.sine_set_total_sweep_time(monophonic_note,  (1.0 - cmd.vel/127.0) * 0.5 + 0.01 )
#					sinemod.sine_set_current_sweep_time(monophonic_note, 0.0)
#					last_frequency = new_frequency
#					last_amplitude = new_amplitude


	if isinstance(cmd, midi.Command.Note_OFF):
		if cmd.chan == trackchan:
			cmd.note+=-24+3+transpose

			if cmd.note in notelist:
				sinemod.sine_set_cont ( notelist[cmd.note], 0 )
				del notelist[cmd.note]

			
#			keycount -= 1
#			if keycount == 0:
#				sinemod.sine_set_cont ( monophonic_note, 0 )
#				monophonic_note = None

capture = midi.Capture(handler)
capture.start()
