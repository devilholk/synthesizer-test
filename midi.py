#encoding=utf-8

in_range = lambda value, start, stop: value >= start and value <= stop

class Command():
	class Start():
		pass

	class Continue():
		pass

	class TimingClock():
		pass

	class Stop():
		pass
		
	class Note_ON():
		def __init__(self, chan, note, vel):
			self.chan, self.note, self.vel = chan, note, vel
	
		def __str__(self):
			return "%s, ch=%i n=%i v=%i"%(self.__class__.__name__, self.chan, self.note, self.vel)	

	class Unknown():
		def __init__(self, *data):
			self.data = data
		
		def __str__(self):
			return "%s, data=%s"%(self.__class__.__name__, str(self.data))	

	class Program_Change():
		def __init__(self, chan, program):
			self.chan, self.program = chan, program

		def __str__(self):
			return "%s, ch=%i p=%i"%(self.__class__.__name__, self.chan, self.program)	

	class Pitch_Wheel():
		def __init__(self, chan, pitch):
			self.chan, self.pitch = chan, pitch
		
	
	class Note_OFF():
		def __init__(self, chan, note):
			self.chan, self.note = chan, note
	
		def __str__(self):
			return "%s, ch=%i n=%i"%(self.__class__.__name__, self.chan, self.note)	


	
import serial, threading

class Capture(threading.Thread):
	def __init__(self, handler, device="/dev/ttyUSB0"):
		threading.Thread.__init__(self)
		self.dev = device
		self.handler = handler
		self.running = False

	def run(self):
		self.running = True
		self.port = serial.Serial(self.dev, baudrate=31250)
		while self.running:
			self.handler( parse_midi( self.port ))

	def stop(self):
		self.running = False
		self.port.close()

def parse_midi(port):
	cmd = ord(port.read(1))
	if in_range(cmd, 0x80, 0x8F):
		return Command.Note_OFF(cmd & 0xF, ord(port.read(1)), ord(port.read(1)))
	elif in_range(cmd, 0x90, 0x9F):
		note, vel = ord(port.read(1)), ord(port.read(1))
		if vel == 0:
			return Command.Note_OFF(cmd & 0xF, note)
		return Command.Note_ON(cmd & 0xF, note, vel)
	elif in_range(cmd, 0xB0, 0xBF) or in_range(cmd, 0x00, 0x7F):
		return Command.Unknown(cmd, ord(port.read(1)), ord(port.read(1)))
	elif cmd == 0xFA:
		return Command.Start
	elif cmd == 0xFB:
		return Command.Continue
	elif cmd == 0xFC:
		return Command.Stop
	elif cmd == 0xF8:
		return Command.TimingClock
	elif in_range(cmd, 0xC0, 0xCF):
		return Command.Program_Change(cmd & 0xF, ord(port.read(1)))		
	elif in_range(cmd, 0xE0, 0xEF):
		return Command.Pitch_Wheel(cmd & 0xF, (ord(port.read(1))<<7) | ord(port.read(1)))		
	
	raise Exception("Vad i helvete? CMD = 0x%x"%cmd)
	

	