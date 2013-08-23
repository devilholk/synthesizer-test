import synth as _synth

class _ListItem(object):
	def __del__(self):
		print "Jag dog!"
	
	def __init__(self, owner):
		self.owner = owner
		print "Yay, jag bor i", owner


class Processor(object):
	pass

class List(list):
	def __init__(self, owner):
		list.__init__(self)
		self.listref = {}
		self.owner = owner

class ProcessorList(List):
	def append(self, processor):
		if not isinstance(processor, Processor):
			raise TypeError("%s.append requires %s"%( self.__class__.__name__, Processor.__name__))
		
		if isinstance(self.owner, Channel):
			self.listref[processor] = _synth.channel_add_processor(self.owner._ref, processor._ref)
		else:
			raise TypeError("%s.append( %s ) of unknown owner object %s"%(self.__class__.__name__, processor, repr(self.owner )))
		
	def remove(self, processor):
		if isinstance(self.owner, Channel):
			_synth.channel_del_processor(self.owner._ref, self.listref[processor])
		else:
			raise TypeError("%s.append( %s ) of unknown owner object %s"%(self.__class__.__name__, processor, repr(self.owner )))
		
					
class NoteList(List):
	def append(self, processor):
		if isinstance(processor, Channel):
			if isinstance(self.owner, Channel):
				self.listref[processor] = _synth.channel_add_note(self.owner._ref, processor._processor)
			else:
				raise TypeError("%s.append( %s ) of unknown owner object %s"%(self.__class__.__name__, processor, repr(self.owner )))
		
		elif isinstance(processor, Processor):
			if isinstance(self.owner, Channel):
				self.listref[processor] = _synth.channel_add_note(self.owner._ref, processor._ref)
			else:
				raise TypeError("%s.append( %s ) of unknown owner object %s"%(self.__class__.__name__, processor, repr(self.owner )))


		else:
			raise TypeError("%s.append requires %s"%( self.__class__.__name__, Processor.__name__))
		
		
	def remove(self, processor):
		if isinstance(self.owner, Channel):
			_synth.channel_del_note(self.owner._ref, self.listref[processor])
		else:
			raise TypeError("%s.append( %s ) of unknown owner object %s"%(self.__class__.__name__, processor, repr(self.owner )))

class Channel(object):
	def __init__(self):
		self._ref = _synth.channel_create()
		self._processor = _synth.channel_create_processor(self._ref)
		self.notes = NoteList( self )
		self.processors = ProcessorList( self )
		
	def __del__(self):
		_synth.channel_destroy(self._processor)
		_synth.channel_destroy(self._ref)
		
	def __getattr__(self, item):
		if item == 'volume':
			return _synth.channel_get(self._ref)[0]
		elif item == 'flag':
			return _synth.channel_get(self._ref)[1]

		return object.__getattr__(self, item)		
				
	def __setattr__(self, item, value):

		if item in ('volume', 'flags'):		
			volume, flags = _synth.channel_get(self._ref)

		if item == 'volume':
			volume = value
			_synth.channel_set(self._ref, volume, flags)
			return
		elif item == 'flags':
			flags = value
			_synth.channel_set(self._ref, volume, flags)
			return
	
		return object.__setattr__(self, item, value)

	def debug_dump(self):
		if hasattr(_synth, "channel_debug"):
			notelistref, processorlistref, bufref, volume, flags = _synth.channel_debug( self._ref )
			notefirst, notelast = _synth.list_debug(notelistref)
			processorfirst, processorlast = _synth.list_debug(processorlistref)

			print "Channel data:"
			print "\tFlags:", flags
			print "\tVolume:", volume
			print "\tNotes:",	
			ptr = notefirst
			if not ptr:
				print "None"
			else:
				print

			while ptr:
				noteitem, notenext, noteprev = _synth.listitem_debug(ptr)
				ptr = notenext
				print "\t\t", hex(noteitem)
			
			print "\tProcessors:",
			ptr = processorfirst
			if not ptr:
				print "None"
			else:
				print
				
			while ptr:
				procitem, procnext, procprev = _synth.listitem_debug(ptr)
				ptr = procnext
				print "\t\t", hex(procitem)
			
		else:
			raise Exception("Recompile with -DDEBUG or -DDEBUG_COLOR to use this functionality.")

#	def debug_dump(self):
#		if hasattr(_synth, "channel_dump"):
#			_synth.channel_dump( self._ref )
#		else:
#			raise Exception("Recompile with -DDEBUG or -DDEBUG_COLOR to use this functionality.")
		
class processors():
	class Test440Low(Processor):
		def __init__(self):
			self._ref = _synth.processor_test_get_sine440_low()
		
		def __del__(self):
			_synth.processor_destroy(self._ref)


	class Generic_Sine(Processor):
		def __init__(self, frequency, amplitude):
			self._ref = _synth.processor_test_generic_sine(frequency, amplitude)
		
		def __del__(self):
			_synth.processor_destroy(self._ref)
	
		def __getattr__(self, item):
			if item == 'frequency':
				return _synth.processor_test_generic_sine_get(self._ref)[0]
			elif item == 'amplitude':
				return _synth.processor_test_generic_sine_get(self._ref)[1]
	
			return object.__getattr__(self, item)		
					
		def __setattr__(self, item, value):
	
			if item in ('frequency', 'amplitude'):		
				frequency, amplitude = _synth.processor_test_generic_sine_get(self._ref)
	
				if item == 'frequency':
					frequency = value
				elif item == 'amplitude':
					amplitude = value
			
				_synth.processor_test_generic_sine_set(self._ref, frequency, amplitude)
		
			return object.__setattr__(self, item, value)



					
def start(processor):
	if isinstance(processor, Processor):
		_synth.init( processor._ref )
	elif isinstance(processor, Channel):
		_synth.init( processor._processor )
	else:
		raise TypeError("start() require %s as argument"%Processor.__name__ )		

def stop():
	print "stop synth not implemented"