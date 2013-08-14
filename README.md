synthesizer-test
================

This is very early in development. How to use it in current state:

1...Run make to compile the module. It depends on portaudio and python 2.7
2...Hook up a MIDI device to a serial port
3...Modify test1.py, provide keyword argument device="/dev/your-serial-device" where it creates midi.Capture
4...Execute test1.py from python 2.7
5...Send MIDI data and listen. Program changes are ignored, a few different instruments exists on different channels, look in test1.py for more information

Notes:

  Allocated resources are _not_ freed now in the library
  This is very early testing!
