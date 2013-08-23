from synthesizer import *

#c = Channel()
#
#subchan = Channel()
#note = Channel()
#
#c.processors.append( subchan )
#c.processors.remove( subchan )
#
#c.notes.append( note )
#c.notes.remove( note )


sine1 = processors.Generic_Sine(400, 0.25)
sine2 = processors.Generic_Sine(600, 0.25)

import synth


channel = Channel()
#ch1= Channel()
#ch2= Channel()

channel.flags = 1

channel.notes.append ( sine1 ) 
#channel.notes.append ( sine2 ) 
#channel.notes.append ( ch2 ) 


#ch1.notes.append(sine1)
#ch2.notes.append(sine2)


start( channel )

f = synth.processor_test_filter(500.0, 0.0, 9)

synth.channel_add_note ( channel._ref, f )

channel.debug_dump()

#raw_input()

#
#
#import time
#import math
#e= 1.0
#while 1:
#	e+=1.0/12.0;
#	if e > 8:
#		e=1.0
#	sine1.frequency = 27.5 * math.pow(2.0, e)
#	print sine1.frequency
#	time.sleep(0.01	)