#encoding=utf-8

import curses

#Init terminal
scr = curses.initscr()
curses.start_color()
curses.noecho()
curses.cbreak()
scr.keypad(True)
curses.curs_set(False)

curses.init_pair(1, curses.COLOR_WHITE, curses.COLOR_GREEN)
curses.init_pair(2, curses.COLOR_CYAN, curses.COLOR_BLACK)

t = scr.subwin(12, 38, 0, 42)
t.box()
t.addstr(0,2,"< MIDI Input >")

import midi


t2=scr.subwin(10, 36, 1, 43)
t2.scrollok(True)

def add_data(msg):
	t2.scroll(1)
	t2.move(9,1)
	for c, tx in msg:
		t2.addstr(tx,curses.color_pair(c))
	t2.refresh()


scr.addstr(0,0,"[ MIDI Synthesizer ]", curses.color_pair(1) | curses.A_BOLD)
#scr.addstr(str(dir(t)))
t.refresh()

def testa(cmd):
	add_data( [(0, str(cmd))] )

capt=midi.Capture(testa)
capt.start()

scr.getch()

capt.running=False
capt.port.close()


#Clean up termina
curses.nocbreak()
scr.keypad(False)
curses.echo()
curses.endwin()

