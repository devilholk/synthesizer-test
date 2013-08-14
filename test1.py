#encoding=utf-8
import synth, midi
synth.init()
synth.create_reverb(8192, 0)

def handler( cmd ):
	if isinstance(cmd, midi.Command.Note_ON):
		cmd.note+=-24+3
#		print cmd
		if cmd.vel:
			cmd.vel*=0.2/127.0
			if cmd.chan==2: 
				cmd.chan=1
			elif cmd.chan==9:
				if cmd.note == 15:
					cmd.chan=6
					cmd.note=15
					cmd.vel*=2.5
				elif cmd.note in (48, 21, 25):
					cmd.chan=4
			elif cmd.chan==11:
				cmd.chan=2
				cmd.note+=12
			elif cmd.chan==10:
				cmd.chan=7
				cmd.note+=24+12
				cmd.vel*=.33
			elif cmd.chan==12:
				cmd.chan=2
				cmd.note+=12+12+12
				cmd.vel*=0.25

			synth.play_note(cmd.chan, cmd.note, cmd.vel )


#midi.capture_midi_in( handler )

capture = midi.Capture(handler)
capture.start()

