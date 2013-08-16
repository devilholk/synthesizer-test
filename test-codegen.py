#encoding=utf-8
import codegen, time

#Play 440 Hz sine on left and 30 Hz triangle on right

f1 = 440.0	
f2 = 30.0
code = {
	'left': 'sin(%s*t*2.0*M_PI) * 0.2'%f1,
	'right': '(fmod(%s*t*2.0,2.0)-1.0) * 0.1'%f2
} 

t = time.time()
fptr = codegen.build_function(code)
print "Compilation and linkage took %s mS" % int((time.time()-t)*1000)
print "Function pointer = 0x%x" % fptr	
print "Playing your function - press enter to stop"

import synth
synth.init(fptr)
raw_input()


print "Attempting to reload module"
fptr = codegen.build_function(code)
print "New ptr = 0x%x"%fptr

raw_input()