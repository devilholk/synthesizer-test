#!/bin/python2 -u
#encoding=utf-8

in_range = lambda value, start, stop: value >= start and value <= stop
	
class cmd_NoData():
	def __str__(self):
		return "%s"%(self.__class__.__name__)	

class cmd_Start(cmd_NoData):
	pass

class cmd_Continue(cmd_NoData):
	pass

class cmd_TimingClock(cmd_NoData):
	pass


class cmd_Stop(cmd_NoData):
	pass
		
class cmd_Note_Vel():
	def __init__(self, chan, note, vel):
		self.chan, self.note, self.vel = chan, note, vel
		
	def __str__(self):
		return "%s, ch=%i n=%i v=%i"%(self.__class__.__name__, self.chan, self.note, self.vel)	

class cmd_Note():
	def __init__(self, chan, note):
		self.chan, self.note = chan, note
		
	def __str__(self):
		return "%s, ch=%i n=%i"%(self.__class__.__name__, self.chan, self.note)	


class cmd_Generic():
	def __init__(self, *data):
		self.data = data
		
	def __str__(self):
		return "%s, data=%s"%(self.__class__.__name__, str(self.data))	

class cmd_Program_Change(cmd_Generic):
	pass

class cmd_Pitch_Wheel_Control(cmd_Generic):
	pass
	
class Note_OFF(cmd_Note):
	pass	
class Note_ON(cmd_Note_Vel):
	pass	
	
import serial

def capture_midi_in(cb):	
	s=serial.Serial("/dev/ttyUSB0", baudrate=31250)
	while 1:
		cmd=ord(s.read(1))
		
		if in_range(cmd, 0x80, 0x8F):
			command = Note_OFF(cmd & 0xF, ord(s.read(1)), ord(s.read(1)))
		elif in_range(cmd, 0x90, 0x9F):
			note, vel = ord(s.read(1)), ord(s.read(1))
			if vel == 0:
				command = Note_OFF(cmd & 0xF, note)
			else:
				command = Note_ON(cmd & 0xF, note, vel)
		elif in_range(cmd, 0xB0, 0xBF):
			command = cmd_Generic(cmd & 0xF, ord(s.read(1)), ord(s.read(1)))
		elif cmd == 0xFA:
			command = cmd_Start
		elif cmd == 0xFB:
			command = cmd_Continue
		elif cmd == 0xFC:
			command = cmd_Stop
		elif cmd == 0xF8:
			command = cmd_TimingClock
		elif in_range(cmd, 0xC0, 0xCF):
			command = cmd_Program_Change(cmd & 0xF, ord(s.read(1)))
	
		elif in_range(cmd, 0x00, 0x7F):		#We don't know but just handle them
			command = cmd_Generic(cmd)
	
		elif cmd == 0x00:
			command = cmd_Generic(cmd)
	
		elif in_range(cmd, 0xE0, 0xEF):
			command = cmd_Pitch_Wheel_Control(cmd & 0xF, ord(s.read(1)), ord(s.read(1)))
		
		else:
			raise Exception("Vad i helvete? CMD = 0x%x"%cmd)
	
		cb(command)
	
		#if command != cmd_TimingClock: 		
			#print command
