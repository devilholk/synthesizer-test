CC = gcc
CFLAGS = -Wall -O2 -march=native -std=gnu99 -fPIC -g
LDFLAGS =  -lpython2.7 -lm -lportaudio -shared
INCLUDES = -I/usr/include/python2.7 -Iinc
SOURCES = synthesizer.c list.c channel.c processor.c processor-test.c filter.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = synth.so

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJECTS)
	rm $(EXECUTABLE)


debug:CFLAGS += -g -DDEBUG_COLOR
debug: $(EXECUTABLE)

debug-macros:CFLAGS += -E -g -DDEBUG_COLOR
debug-macros: $(OBJECTS)